// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function testReadProperty(that, prop, expected) {
  try {
    var actual = that[prop];
    if (actual == expected) {
      app.alert('PASS: ' + prop + ' = ' + actual);
    } else {
      app.alert('FAIL: ' + prop + ' = ' + actual + ', expected = ' + expected);
    }
  } catch (e) {
    app.alert('ERROR: ' + e.toString());
  }
}

function testUnreadableProperty(that, prop) {
  try {
    var value = that[prop];
    app.alert('FAIL: ' + prop + ', expected to throw');
  } catch (e) {
    app.alert('PASS: ' + prop + ' threw ' + e.toString());
  }
}

function testWriteProperty(that, prop, newValue) {
  try {
    that[prop] = newValue;
    var actual = that[prop];
    if (actual == newValue) {
      app.alert('PASS: ' + prop + ' = ' + actual);
    } else {
      app.alert('FAIL: ' + prop + ' = ' + actual + ', expected = ' + newValue);
    }
  } catch (e) {
    app.alert('ERROR: ' + e.toString());
  }
}

function testWriteIgnoredProperty(that, prop, expected, newValue) {
  try {
    that[prop] = newValue;
    var actual = that[prop];
    if (actual == expected) {
      app.alert('PASS: ' + prop + ' = ' + actual);
    } else {
      app.alert('FAIL: ' + prop + ' = ' + actual + ', expected = ' + expected);
    }
  } catch (e) {
    app.alert('ERROR: ' + e.toString());
  }
}

function testUnwritableProperty(that, prop, newValue) {
  try {
    that[prop] = newValue;
    app.alert('FAIL: ' + prop + ' = ' + newValue + ', expected to throw');
  } catch (e) {
    app.alert('PASS: ' + prop + ' threw ' + e.toString());
  }
}

function testRWProperty(that, prop, expected, newValue) {
  testReadProperty(that, prop, expected);
  testWriteProperty(that, prop, newValue);
}

function testRIProperty(that, prop, expected, newValue) {
  testReadProperty(that, prop, expected);
  testWriteIgnoredProperty(that, prop, expected, newValue);
}

function testROProperty(that, prop, expected) {
  testReadProperty(that, prop, expected);
  testUnwritableProperty(that, prop, 42);
}

function testXXProperty(that, prop) {
  testUnreadableProperty(that, prop);
  testUnwritableProperty(that, prop, 42);
}
