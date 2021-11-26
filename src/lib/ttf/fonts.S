
.section .rodata

.balign 8
.global _font_roboto_start
_font_roboto_start:
.incbin "src/lib/ttf/Roboto-Regular.ttf"
.global _font_roboto_end
_font_roboto_end:

.balign 8
.global _font_lato_start
_font_lato_start:
.incbin "src/lib/ttf/Lato-Regular.ttf"
.global _font_lato_end
_font_lato_end:
