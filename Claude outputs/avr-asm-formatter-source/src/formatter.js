// Core formatting logic — pure functions, no VS Code dependency, so it can be
// unit-tested with plain Node before being wired into the extension.
//
// Design goal: never rewrite the INSIDE of a token. AVR indirect addressing
// (X+, -Y, Z+3) uses '+'/'-' as part of the token itself, not as a spaced
// operator. Off-the-shelf formatters that "normalize spacing around + and -"
// corrupt this syntax. This formatter only ever inserts/removes whitespace
// *between* fields (label / mnemonic / operands / comment) and *between*
// comma-separated operands — it never touches characters inside a token.

'use strict';

const DEFAULT_OPTIONS = {
  mnemonicColumn: 4,
  operandColumn: 12,
  commentColumn: 40,
  commentMinSpacing: 2,
  indentStyle: 'spaces', // 'spaces' | 'tabs'
  alignCommentOnlyLines: true,
  mnemonicCase: 'preserve', // 'preserve' | 'upper' | 'lower'
};

/**
 * Split a raw line into { code, comment } where `comment` starts at the
 * first unquoted ';' (or null if there isn't one). Quote-aware so that
 * `.db "a;b"` doesn't get cut in half.
 */
function splitComment(line) {
  let inString = null; // '"' or "'" while inside a string, else null
  for (let i = 0; i < line.length; i++) {
    const ch = line[i];
    if (inString) {
      if (ch === '\\' && inString === '"') {
        i++; // skip escaped char inside double-quoted strings
        continue;
      }
      if (ch === inString) inString = null;
      continue;
    }
    if (ch === '"' || ch === "'") {
      inString = ch;
      continue;
    }
    if (ch === ';') {
      return { code: line.slice(0, i), comment: line.slice(i) };
    }
  }
  return { code: line, comment: null };
}

/**
 * Split an operand string on top-level commas (quote-aware). Never trims or
 * touches characters *inside* an operand beyond surrounding whitespace.
 */
function splitOperands(text) {
  const parts = [];
  let current = '';
  let inString = null;
  for (let i = 0; i < text.length; i++) {
    const ch = text[i];
    if (inString) {
      current += ch;
      if (ch === '\\' && inString === '"') {
        i++;
        if (i < text.length) current += text[i];
        continue;
      }
      if (ch === inString) inString = null;
      continue;
    }
    if (ch === '"' || ch === "'") {
      inString = ch;
      current += ch;
      continue;
    }
    if (ch === ',') {
      parts.push(current.trim());
      current = '';
      continue;
    }
    current += ch;
  }
  if (current.trim() !== '' || parts.length > 0) parts.push(current.trim());
  return parts.filter((p) => p !== '');
}

const LABEL_RE = /^([A-Za-z_.$][\w.$]*)\s*:\s*(.*)$/s;

/**
 * Parse one source line into a structured record. Returns one of:
 *  - { kind: 'blank' }
 *  - { kind: 'commentOnly', comment }
 *  - { kind: 'labelOnly', label, comment }
 *  - { kind: 'code', label|null, mnemonic, operands: string[], comment|null }
 */
function parseLine(rawLine) {
  if (rawLine.trim() === '') return { kind: 'blank' };

  const { code, comment } = splitComment(rawLine);
  const trimmedCode = code.trim();

  if (trimmedCode === '') {
    return { kind: 'commentOnly', comment: comment.trim() };
  }

  let label = null;
  let rest = trimmedCode;
  const m = trimmedCode.match(LABEL_RE);
  if (m) {
    label = m[1];
    rest = m[2].trim();
  }

  if (rest === '') {
    return { kind: 'labelOnly', label, comment: comment ? comment.trim() : null };
  }

  const mm = rest.match(/^(\S+)\s*(.*)$/s);
  const mnemonic = mm[1];
  const operandsRaw = mm[2].trim();
  const operands = operandsRaw === '' ? [] : splitOperands(operandsRaw);

  return {
    kind: 'code',
    label,
    mnemonic,
    operands,
    comment: comment ? comment.trim() : null,
  };
}

function applyCase(mnemonic, mode) {
  if (mode === 'upper') return mnemonic.toUpperCase();
  if (mode === 'lower') return mnemonic.toLowerCase();
  return mnemonic;
}

function padTo(current, targetCol, minSpaces) {
  const min = minSpaces == null ? 1 : minSpaces;
  if (current.length + min > targetCol) {
    return current + ' '.repeat(min);
  }
  return current + ' '.repeat(targetCol - current.length);
}

/**
 * Render a parsed line back to text using the given options. Returns null
 * when the line should be left completely untouched (not currently used,
 * reserved for future "don't touch this" escape hatches).
 */
function renderLine(parsed, options) {
  const opts = Object.assign({}, DEFAULT_OPTIONS, options);

  if (parsed.kind === 'blank') return '';

  if (parsed.kind === 'commentOnly') {
    if (!opts.alignCommentOnlyLines) return parsed.comment;
    return ' '.repeat(opts.mnemonicColumn) + parsed.comment;
  }

  if (parsed.kind === 'labelOnly') {
    let line = parsed.label + ':';
    if (parsed.comment) {
      line = padTo(line, opts.commentColumn, opts.commentMinSpacing) + parsed.comment;
    }
    return line;
  }

  // kind === 'code'
  let line = parsed.label ? parsed.label + ':' : '';
  line = padTo(line, opts.mnemonicColumn, 1);
  line += applyCase(parsed.mnemonic, opts.mnemonicCase);

  if (parsed.operands.length > 0) {
    line = padTo(line, opts.operandColumn, 1);
    line += parsed.operands.join(', ');
  }

  if (parsed.comment) {
    line = padTo(line, opts.commentColumn, opts.commentMinSpacing);
    line += parsed.comment;
  }

  return line;
}

function formatLine(rawLine, options) {
  return renderLine(parseLine(rawLine), options);
}

/**
 * Format an array of lines (no trailing newlines) and return the new array.
 * Pure function — no VS Code dependency — so callers only need to diff
 * against the original per line to build minimal-edit TextEdits.
 */
function formatLines(lines, options) {
  return lines.map((l) => formatLine(l, options));
}

module.exports = {
  DEFAULT_OPTIONS,
  splitComment,
  splitOperands,
  parseLine,
  renderLine,
  formatLine,
  formatLines,
};
