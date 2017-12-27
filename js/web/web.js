'use strict';

const {decode, splitReports, encode, decodeAll} = require('../');

document.getElementById('button').addEventListener('click', () => {
  let input = document.getElementById('input').value;
  if ((input.length % 2) != 0) input = input.substr(0, input.length - 1)
  const binary_input = new Buffer(input, 'hex');

  const output = decodeAll(binary_input);
  const json_output = JSON.stringify(output);

  document.getElementById('output').value = json_output;
});
