/*
  The MIT License

  Copyright (c) 2016 Thomas Sarlandie thomas@sarlandie.net

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <KBoxLogging.h>
#include <KBoxHardware.h>
#include "TaskManager.h"
#include "stats/KBoxMetrics.h"

void TaskManager::addTask(Task* task) {
  if (running) {
    task->setup();
    restartStats();
  }
  tasks.add(task);
}

void TaskManager::setup() {
  for (LinkedList<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++) {
    (*it)->setup();
  }
  restartStats();
}

static Task *lastRanTask;

void TaskManager::loop() {
  elapsedMicros loopTimer;

  int i = 0;
  for (LinkedList<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++, i++) {
    if ((*it)->ready()) {
      elapsedMicros timer;

      // This is useful when debugging and the stack is corrupted, it
      // will always point to the culprit.
      lastRanTask = (*it);

      (*it)->loop();
      if (taskStats) {
        taskStats[i].recordRun(timer);
      }
    }
  }
  loopStats.recordRun(loopTimer);
  KBoxMetrics.metric(KBoxMetricTaskManagerLoopUS, loopTimer);

  if (statDisplayTimer > statDisplayInterval) {
    displayStats();
    statDisplayTimer = 0;
    restartStats();
  }
}

void TaskManager::displayStats() {
  // FIXME: Should upgrade all of this to use KBoxMetrics instead.
  INFO("KBox uptime: %lus RAM Used: %d bytes Free: %d bytes", uptime / 1000, KBox.getUsedRam(), KBox.getFreeRam());
  INFO("-------------------------------------------------------------------------------------");
  int i = 0;
  INFO("%2s %16s %10s %10s %9s %9s %9s", "ID", "TaskName", "Runs", "Total (ms)", "Average (us)", "Min (us)", "Max (us)");
  for (LinkedList<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++) {
    INFO("%2i %16s %10lu %10lu %9lu %9lu %9lu",
        i, (*it)->getTaskName(), taskStats[i].count(), taskStats[i].totalTime() / 1000,
        taskStats[i].avgTime(), taskStats[i].minTime(), taskStats[i].maxTime());
    i++;
  }
  INFO("-- %16s %10lu %10lu %9lu %9lu %9lu", "totals (in ms)",
      loopStats.count(), loopStats.totalTime() / 1000, loopStats.avgTime() / 1000, loopStats.minTime() / 1000, loopStats.maxTime() / 1000);

  INFO("-------------------------------------------------------------------------------------");
}

void TaskManager::restartStats() {
  if (taskStats) {
    delete[] taskStats;
  }
  loopStats = RunStat();
  taskStats = new RunStat[tasks.size()];
}
