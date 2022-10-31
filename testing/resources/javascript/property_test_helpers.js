// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Treat two distinct objects with the same members as equal, e.g. in JS
// ([0, 1] == [0, 1]) evaluates to false. Without this function, we can't
// supply an expected result of object/array type for our tests.
function kindaSortaEqual(arg1, arg2) {
  if (arg1 == arg2) {
    return true;
  }
  if (typeof arg1 != "object") {
    return false;
  }
  if (typeof arg2 != "object") {
    return false;
  }
  return arg1.toString() == arg2.toString();
}

function testReadProperty(that, prop, expected) {
  try {
    var actual = that[prop];
    if (kindaSortaEqual(actual, expected)) {
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
    if (kindaSortaEqual(actual, newValue)) {
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
    if (kindaSortaEqual(actual, expected)) {
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
