# AVR ASM Formatter (personal)

Formateador de columnas para ensamblador AVR (`label:` / mnemónico / operandos / `; comentario`), hecho a la medida porque ninguna extensión del marketplace maneja bien el direccionamiento indirecto de AVR (`X+`, `-Y`, `Z+q`).

## Por qué existe

Los formateadores genéricos que "normalizan espacios alrededor de + y -" rompen `X+`/`-Z` porque no saben que ahí `+`/`-` es parte del modo de direccionamiento, no un operador aritmético. Este formateador nunca reescribe el interior de un token — solo ajusta espacios *entre* campos (label, mnemónico, operandos, comentario) y entre operandos separados por coma.

## Uso

Con un archivo `.asm`/`.inc`/`.s` abierto: `Shift+Alt+F` (Format Document) o Ctrl+Shift+P → "AVR ASM: Format Document". También funciona con selección parcial (Format Selection) y con "Format on Save" si lo activas en tus settings para estos archivos.

## Configuración (settings.json)

```jsonc
"avrAsmFormatter.filePatterns": ["**/*.asm", "**/*.inc", "**/*.s", "**/*.S"],
"avrAsmFormatter.mnemonicColumn": 4,
"avrAsmFormatter.operandColumn": 12,
"avrAsmFormatter.commentColumn": 40,
"avrAsmFormatter.commentMinSpacing": 2,
"avrAsmFormatter.alignCommentOnlyLines": true,
"avrAsmFormatter.mnemonicCase": "preserve" // "preserve" | "upper" | "lower"
```

Ajusta las columnas a lo que se vea más parecido a AVR Studio en tu pantalla/tamaño de fuente.

## Qué garantiza

- Nunca toca caracteres dentro de un operando (`X+`, `-Z`, `Y+3`, `0x1F`, cadenas entre comillas quedan intactas).
- `;` y `,` dentro de cadenas (`"texto; con punto y coma"`) no se confunden con comentarios/separadores.
- Líneas en blanco se quedan realmente en blanco (sin tabs fantasma).
- `label: instruccion` en la misma línea se separa correctamente.
- Si una línea ya es más larga que la columna configurada, nunca genera padding negativo — cae a un espaciado mínimo en vez de romperse.

## Pruebas

`node test/format.test.js` corre la batería de casos (direccionamiento indirecto, comentarios con comillas, overflow de columnas, etc.) sin necesidad de abrir VS Code.
