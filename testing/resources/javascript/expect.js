function expect(str, expected) {
  try {
    var result = eval(str);
    if (result == expected) {
      app.alert('PASS: ' + str + ' = ' + result);
    } else {
      app.alert('FAIL: ' + str + ' = ' + result + ', expected = ' + expected);
    }
  } catch (e) {
    app.alert('ERROR: ' + e.toString());
  }
}

function expectError(str) {
  try {
    var result = eval(str);
    app.alert('FAIL: ' + str + ' = ' + result + ', expected to throw error');
  } catch (e) {
    app.alert('PASS: ' + str + ' threw error ' + e.toString());
  }
}
