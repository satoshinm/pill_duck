'use strict';

const test = require('tape');
const {decode, splitReports, encode, decodeAll} = require('./');

test('decode mouse', (t) => {
  const buf = new Buffer("0200010000", "hex");

  t.deepEqual(decode(buf), { report_id: 2, buttons: 0, x: 0, y: 1, wheel: 0 });

  t.end();
});

test('decode keyboard', (t) => {
  const buf = new Buffer("0102010b0000000000", "hex");

  console.log(decode(buf));
  t.deepEqual(decode(buf), { report_id: 1, modifiers: 2, reserved: 1, keys_down: [ 11, 0, 0, 0, 0, 0 ], leds: 0 });
    
  t.end();
});

test('decode hello world', (t) => {
  const bufs = new Buffer(
"0102010b000000000000000000000000"+
"01000100000000000000000000000000"+
"01000108000000000000000000000000"+
"01000100000000000000000000000000"+
"0100010f000000000000000000000000"+
"01000100000000000000000000000000"+
"0100010f000000000000000000000000"+
"01000100000000000000000000000000"+
"01000112000000000000000000000000"+
"01000100000000000000000000000000"+
"01000136000000000000000000000000"+
"01000100000000000000000000000000"+
"0100012c000000000000000000000000"+
"01000100000000000000000000000000"+
"0100011a000000000000000000000000"+
"01000100000000000000000000000000"+
"01000112000000000000000000000000"+
"01000100000000000000000000000000"+
"01000115000000000000000000000000"+
"01000100000000000000000000000000"+
"0100010f000000000000000000000000"+
"01000100000000000000000000000000"+
"01000107000000000000000000000000"+
"01000100000000000000000000000000"+
"ff000000000000000000000000000000", "hex");

  const expected = [
{ report_id: 1, modifiers: 2, reserved: 1, keys_down: [ 11, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 8, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 15, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 15, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 18, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 54, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 44, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 26, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 18, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 21, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 15, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 7, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 255 }];

  t.deepEqual(decodeAll(bufs), expected);
  t.end();
});

test('encode hello world', (t) => {
  const array = [
{ report_id: 1, modifiers: 2, reserved: 1, keys_down: [ 11, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 8, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 15, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 15, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 18, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 54, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 44, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 26, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 18, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 21, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 15, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 7, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 1, modifiers: 0, reserved: 1, keys_down: [ 0, 0, 0, 0, 0, 0 ], leds: 0 },
{ report_id: 255 }];
  let bufs = "";
  for (let i = 0; i < array.length; ++i) {
    const buf = encode(array[i]);
    bufs += buf.toString('hex')
  }
  t.equal(bufs, "0102010b0000000000000000000000000100010000000000000000000000000001000108000000000000000000000000010001000000000000000000000000000100010f000000000000000000000000010001000000000000000000000000000100010f00000000000000000000000001000100000000000000000000000000010001120000000000000000000000000100010000000000000000000000000001000136000000000000000000000000010001000000000000000000000000000100012c000000000000000000000000010001000000000000000000000000000100011a00000000000000000000000001000100000000000000000000000000010001120000000000000000000000000100010000000000000000000000000001000115000000000000000000000000010001000000000000000000000000000100010f00000000000000000000000001000100000000000000000000000000010001070000000000000000000000000100010000000000000000000000000000000000000000000000000000000000");
  t.end();
});
