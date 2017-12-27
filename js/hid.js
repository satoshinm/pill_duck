'use strict';

const REPORT_ID_KEYBOARD = 1;
const REPORT_ID_MOUSE = 2;

// Decode USB HID composite device with 1-byte report ID prefix, and either mouse or keyboard
function decode(buf) {
  const result = {};
  const report_id = buf.readUInt8(0);
  result.report_id = report_id;

  if (report_id === REPORT_ID_MOUSE) {
    result.buttons = buf.readUInt8(1);
    result.x = buf.readUInt8(1);
    result.y = buf.readUInt8(2);
    result.wheel = buf.readUInt8(3);
  } else if (report_id === REPORT_ID_KEYBOARD) {
    result.modifiers = buf.readUInt8(1);
    result.reserved = buf.readUInt8(2);
    result.keys_down = [];
    for (let i = 0; i < 6; ++i) {
      result.keys_down.push(buf.readUInt8(3 + i));
    }
    result.leds = buf.readUInt8(8);
  } else {
  }

  return result;
}

function encode(result) {
  const buf = Buffer.alloc(16);

  if (result.report_id == REPORT_ID_MOUSE) {
    buf.writeUInt8(REPORT_ID_MOUSE, 0);
    buf.writeUInt8(result.buttons, 1);
    buf.writeUInt8(result.x, 2);
    buf.writeUInt8(result.y, 3);
    buf.writeUInt8(result.wheel, 4);
  } else if (result.report_id == REPORT_ID_KEYBOARD) {
    buf.writeUInt8(REPORT_ID_KEYBOARD, 0);
    buf.writeUInt8(result.modifiers, 1);
    buf.writeUInt8(result.reserved, 2);
    for (let i = 0; i < 6; ++i) {
      buf.writeUInt8(result.keys_down[i], 3 + i);
    }
    buf.writeUInt8(result.leds, 8);
  }

  return buf;
}

// Split fixed-size report records into an array of each
function splitReports(bufs) {
  const array = [];
  let i = 0;
  do {
    const buf = bufs.slice(i, (i + 1) * 16);
    array.push(buf);
    i += 16;
  } while (i < bufs.length);
  return array;
}

module.exports = {decode, splitReports, encode};
