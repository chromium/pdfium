// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function expect(expression, expected) {
  try {
    var actual = eval(expression);
    if (actual == expected) {
      app.alert('PASS: ' + expression + ' = ' + actual);
    } else {
      app.alert('FAIL: ' + expression + ' = ' + actual + ', expected ' + expected + " ");
    }
  } catch (e) {
    app.alert('ERROR: ' + e);
  }
}

function expectError(expression) {
  try {
    var actual = eval(expression);
    app.alert('FAIL: ' + expression + ' = ' + actual + ', expected to throw');
  } catch (e) {
    app.alert('PASS: ' + expression + ' threw ' + e);
  }
}
