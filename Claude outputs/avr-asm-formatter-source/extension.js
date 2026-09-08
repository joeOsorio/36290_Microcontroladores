'use strict';

const vscode = require('vscode');
const { formatLine } = require('./src/formatter');

function readOptions() {
  const cfg = vscode.workspace.getConfiguration('avrAsmFormatter');
  return {
    mnemonicColumn: cfg.get('mnemonicColumn'),
    operandColumn: cfg.get('operandColumn'),
    commentColumn: cfg.get('commentColumn'),
    commentMinSpacing: cfg.get('commentMinSpacing'),
    alignCommentOnlyLines: cfg.get('alignCommentOnlyLines'),
    mnemonicCase: cfg.get('mnemonicCase'),
  };
}

function buildSelector() {
  const cfg = vscode.workspace.getConfiguration('avrAsmFormatter');
  const patterns = cfg.get('filePatterns') || ['**/*.asm', '**/*.inc', '**/*.s', '**/*.S'];
  return patterns.map((pattern) => ({ scheme: 'file', pattern }));
}

function editsForLines(document, startLine, endLine, options) {
  const edits = [];
  for (let i = startLine; i <= endLine; i++) {
    const line = document.lineAt(i);
    const newText = formatLine(line.text, options);
    if (newText !== line.text) {
      edits.push(vscode.TextEdit.replace(line.range, newText));
    }
  }
  return edits;
}

function activate(context) {
  let registrations = [];

  function register() {
    for (const r of registrations) r.dispose();
    registrations = [];

    const selector = buildSelector();

    registrations.push(
      vscode.languages.registerDocumentFormattingEditProvider(selector, {
        provideDocumentFormattingEdits(document) {
          const options = readOptions();
          return editsForLines(document, 0, document.lineCount - 1, options);
        },
      })
    );

    registrations.push(
      vscode.languages.registerDocumentRangeFormattingEditProvider(selector, {
        provideDocumentRangeFormattingEdits(document, range) {
          const options = readOptions();
          return editsForLines(document, range.start.line, range.end.line, options);
        },
      })
    );

    for (const r of registrations) context.subscriptions.push(r);
  }

  register();

  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration('avrAsmFormatter.filePatterns')) {
        register();
      }
    })
  );

  context.subscriptions.push(
    vscode.commands.registerCommand('avrAsmFormatter.formatDocument', async () => {
      const editor = vscode.window.activeTextEditor;
      if (!editor) return;
      const options = readOptions();
      const edits = editsForLines(editor.document, 0, editor.document.lineCount - 1, options);
      await editor.edit((editBuilder) => {
        for (const e of edits) editBuilder.replace(e.range, e.newText);
      });
    })
  );
}

function deactivate() {}

module.exports = { activate, deactivate };
