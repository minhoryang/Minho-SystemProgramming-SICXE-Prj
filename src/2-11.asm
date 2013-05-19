5 copy start 0
10 first stl retadr
15 cloop jsub rdrec
20 lda length
25 comp #0
30 jeq endfil
35 jsub wrred
40 j cloop
45 endfil lda =C'EOF'
50 sta BUFFER
55 LDA #3
60 sta length
65 jsub wrrec
70 j @retadr
92 use CDATA
95 retadr resw 1
100 length resw 1
103 use cblks
105 buffer resb 4096
106 bufend equ *
107 maxlen equ bufend-buffer
110 .
115 .
120 .
123 use
125 rdrec clear x
130 clear a
132 clear s
133 +lDT #MAXLEN
13 rloop td input
140 jeq rloop
145 rd input
150 compr a, s
155 jeq exit
160 stch buffer, x
165 tixr t
170 jlt rloop
175 exit stx length
180 rsub
173 use cdata
185 input byte X'F1'
195 .
200 .
205 .
208 use
210 wrrec clear x
212 ldt length
215 wloop td =X'05'
220 jeq wloop
225 ldch buffer, x
230 wd =X'05'
235 tixr t
240 jlt wloop
245 rsub
252 use cdata
253 ltorg
255 end first
