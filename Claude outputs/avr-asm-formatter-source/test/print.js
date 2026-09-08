'use strict';
const { formatLine, formatLines } = require('../src/formatter');

const opts = {
  mnemonicColumn: 4,
  operandColumn: 12,
  commentColumn: 32,
  commentMinSpacing: 2,
  alignCommentOnlyLines: true,
  mnemonicCase: 'preserve',
};

const cases = [
  '    ld r16,X+',
  'ld r17,   -Z',
  'ldd   r18, Y+3',
  'end: rjmp end',
  'start:',
  '  ldi r16,0x1F ;mask',
  '   ; full line comment',
  '',
  '.db "hi, there; not a comment", 0',
  'movl $10, %eax   # not a semicolon comment, stays literal',
  'ldr r0, [r1], #4',
  '   LDI   R16,   0x1F',
];

for (const c of cases) {
  console.log(JSON.stringify(c), '->', JSON.stringify(formatLine(c, opts)));
}

console.log('--- upper case mnemonic ---');
console.log(JSON.stringify(formatLine('ldi r16,0x1F', Object.assign({}, opts, { mnemonicCase: 'upper' }))));

console.log('--- full document (blank line preserved) ---');
const doc = ['mainloop:', 'ldi   r17,   6', '', '     cpi r18,4', 'brne skip'];
console.log(formatLines(doc, opts).map((l) => JSON.stringify(l)).join('\n'));
