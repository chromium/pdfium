// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function testLegalConstructor(name, allowed) {
  const constructorString = name + ".constructor";
  var constructor;
  try {
    constructor = eval(constructorString);
  } catch (e) {
    app.alert("FAIL: No such " + constructorString);
    return;
  }
  try {
    constructor();
    app.alert("FAIL: " + constructorString + "(): returned");
  } catch (e) {
    app.alert("PASS: " + constructorString + "(): " + e);
  }
  try {
    var thing = new constructor;
    app.alert("PASS: new " + constructorString + ": " + thing);
  } catch (e) {
    app.alert("FAIL: new " + constructorString + ": " + e);
  }
}

function testIllegalConstructor(name, allowed) {
  const constructorString = name + ".constructor";
  var constructor;
  try {
    constructor = eval(constructorString);
  } catch (e) {
    app.alert("FAIL: No such " + constructorString);
    return;
  }
  try {
    constructor();
    app.alert("FAIL: " + constructorString + "(): returned");
  } catch (e) {
    app.alert("PASS: " + constructorString + "(): " + e);
  }
  try {
    new constructor;
    app.alert("FAIL: new " + constructorString + ": returned");
  } catch (e) {
    app.alert("PASS: new " + constructorString + ": " + e);
  }
}
