/*
     __  __     ______     ______     __  __
    /\ \/ /    /\  == \   /\  __ \   /\_\_\_\
    \ \  _"-.  \ \  __<   \ \ \/\ \  \/_/\_\/_
     \ \_\ \_\  \ \_____\  \ \_____\   /\_\/\_\
       \/_/\/_/   \/_____/   \/_____/   \/_/\/_/

  The MIT License

  Copyright (c) 2017 Thomas Sarlandie thomas@sarlandie.net

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

#include <stdio.h>
#include "common/signalk/SKUpdate.h"
#include "IMUMonitorPage.h"
#include "common/config/KBoxConfig.h"
#include <KBoxLogging.h>
#include "signalk/SKUnits.h"


IMUMonitorPage::IMUMonitorPage(SKHub& hub, IMUService &imuService) {
  static const int col1 = 5;
  static const int col2 = 200;
  static const int row1 = 26;
  static const int row2 = 50;
  static const int row3 = 152;
  static const int row4 = 182;

  addLayer(new TextLayer(Point(col1, row1), Size(20, 20), "HDG ° Mag", ColorWhite, ColorBlack, FontDefault));
  addLayer(new TextLayer(Point(col2, row1), Size(20, 20), "Calibration", ColorWhite, ColorBlack, FontDefault));
  addLayer(new TextLayer(Point(col1, row3), Size(20, 20), "Heel °", ColorWhite, ColorBlack, FontDefault));
  addLayer(new TextLayer(Point(col2, row3), Size(20, 20), "Pitch °", ColorWhite, ColorBlack, FontDefault));

  _hdgTL = new TextLayer(Point(col1, row2), Size(20, 20), "--", ColorWhite, ColorBlack, FontLarge);
  _calTL = new TextLayer(Point(col2, row2), Size(20, 20), "--", ColorWhite, ColorBlack, FontLarge);
  _heelTL  = new TextLayer(Point(col1, row4), Size(20, 20), "--", ColorWhite, ColorBlack, FontLarge);
  _pitchTL = new TextLayer(Point(col2, row4), Size(20, 20), "--", ColorWhite, ColorBlack, FontLarge);

  addLayer(_hdgTL);
  addLayer(_calTL);
  addLayer(_heelTL);
  addLayer(_pitchTL);

  hub.subscribe(this);

  imuService.getLastValues(_accelCalibration, _pitch, _heel, _magCalibration, _heading);
  DEBUG("AccelCalibration: %i | MagCalibration: %i", _accelCalibration, _magCalibration);
}


void IMUMonitorPage::updateReceived(const SKUpdate& up) {



  // No Updates coming when Calib below cfIMU_MIN_CAL
  // Aproach was for trusted values only
  // Disadvantage is, that the display is stuck to the last value!

  if ( up.hasNavigationHeadingMagnetic() ) {
    const SKValue& vm = up.getNavigationHeadingMagnetic();
    // DEBUG( "Heading Magnetic %f", SKRadToDeg( vm.getNumberValue() ) );
    _hdgTL->setText(String( SKRadToDeg( vm.getNumberValue() ), 1) + "°        ");
  }


  // Always show Hdg from IMU-sensor, but if the value is not trusted change color to Red
  // TODO: Some damping for the display
	/*
  _hdgTL->setText(String( IMUService::IMU_HdgFiltered, 1) + "°        ");
  _calTL->setText(String( IMUService::magCAL) + "   ");

	if ( IMUService::magCAL < cfHdgMinCal ) {
    _hdgTL->setColor(ColorRed);
    _calTL->setColor(ColorRed);
  } else {
    _hdgTL->setColor(ColorWhite);
    _calTL->setColor(ColorWhite);
  };
	 */

  // GyroCalib is more likely better than magCalib, so we hope there is an update
  // from IMUService
  // TODO: An own Visitor for Display Values (damped values with lower frequency)
  if ( up.hasNavigationAttitude() ) {
    const SKValue& vm = up.getNavigationAttitude();
    //DEBUG("MagCAL from IMUService: %i ", IMUService::magCAL );
    _pitchTL->setText(String( SKRadToDeg( vm.getAttitudeValue().pitch ), 1) + "°     ");
    _heelTL->setText(String( SKRadToDeg( vm.getAttitudeValue().roll ), 1) + "°     ");
  } else {
    //DEBUG("up.hasNavigationAttitude() == FALSE");
  }
}