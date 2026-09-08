'use strict';
const assert = require('assert');
const { formatLine, formatLines } = require('../src/formatter');

const opts = {
  mnemonicColumn: 4,
  operandColumn: 12,
  commentColumn: 32,
  commentMinSpacing: 2,
  alignCommentOnlyLines: true,
  mnemonicCase: 'preserve',
};

let failures = 0;
function check(name, actual, expected) {
  try {
    assert.strictEqual(actual, expected);
  } catch (e) {
    failures++;
    console.error(`FAIL: ${name}`);
    console.error(`  expected: ${JSON.stringify(expected)}`);
    console.error(`  actual:   ${JSON.stringify(actual)}`);
  }
}

// --- The core safety property: never rewrite characters INSIDE a token.
// AVR post-increment / pre-decrement / displacement addressing must survive
// byte-for-byte, unlike generic "space out +/-" formatters. ---
check('post-increment X+ untouched', formatLine('    ld r16,X+', opts), '    ld      r16, X+');
check('pre-decrement -Z untouched', formatLine('ld r17,   -Z', opts), '    ld      r17, -Z');
check('displacement Y+3 untouched', formatLine('ldd   r18, Y+3', opts), '    ldd     r18, Y+3');

// --- label + instruction on the SAME line gets split into label/mnemonic ---
check('label + instruction same line', formatLine('end: rjmp end', opts), 'end: rjmp   end');

// --- label alone on its own line stays untouched ---
check('label-only line', formatLine('start:', opts), 'start:');

// --- comments: preserved verbatim after ';', aligned to commentColumn ---
check(
  'inline comment aligned',
  formatLine('  ldi r16,0x1F ;mask', opts),
  '    ldi     r16, 0x1F           ;mask'
);
check(
  'full-line comment re-indented to mnemonic column',
  formatLine('   ; full line comment', opts),
  '    ; full line comment'
);

// --- blank lines must stay truly empty, never gain stray whitespace ---
check('blank line stays blank', formatLine('', opts), '');

// --- quote-awareness: ';' and ',' inside a string are not delimiters ---
check(
  'semicolon/comma inside string literal preserved',
  formatLine('.db "hi, there; not a comment", 0', opts),
  '    .db     "hi, there; not a comment", 0'
);

// --- graceful degradation when label/operands overflow a column: never negative-pad ---
check(
  'long label falls back to single space, never overlaps',
  formatLine('averylonglabelname: ldi r16,0x1F ; comment', opts),
  'averylonglabelname: ldi r16, 0x1F  ; comment'
);
check(
  'operands overflowing comment column fall back to commentMinSpacing',
  formatLine('ldi r16, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF ; long', opts),
  '    ldi     r16, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF  ; long'
);

// --- mnemonic-only line (no operands, no comment) ---
check('bare mnemonic', formatLine('reti', opts), '    reti');

// --- mnemonicCase option only touches the mnemonic, never operands ---
check(
  'uppercase mnemonic leaves operands alone',
  formatLine('ldi r16,0x1F', Object.assign({}, opts, { mnemonicCase: 'upper' })),
  '    LDI     r16, 0x1F'
);

// --- full document: blank lines preserved across a formatLines() pass ---
check(
  'formatLines preserves blank lines in a document',
  formatLines(['mainloop:', 'ldi   r17,   6', '', '     cpi r18,4', 'brne skip'], opts).join('\n'),
  ['mainloop:', '    ldi     r17, 6', '', '    cpi     r18, 4', '    brne    skip'].join('\n')
);

if (failures > 0) {
  console.error(`\n${failures} test(s) failed.`);
  process.exit(1);
} else {
  console.log('All formatter tests passed.');
}
