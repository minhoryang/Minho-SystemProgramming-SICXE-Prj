  5 COPY   START  0
  6        EXTDEF BUFFER, BUFEND, LENGTH
  7        EXTREF RDREC, WRREC
 10 FIRST  STL    RETADR
 15 CLOOP  +JSUB  RDREC
 20        LDA    LENGTH
 25        COMP   #0
 30        JEQ    ENDFIL
 35        +JSUB  WRREC
 40        J      CLOOP
 45 ENDFIL LDA    =C'EOF'
 50        STA    BUFFER
 55        LDA    #3
 60        STA    LENGTH
 65        +JSUB  WRREC
 70        J      @RETADR
 95 RETADR RESW   1
100 LENGTH RESW   1
103        LTORG
105 BUFFER RESB   4096
106 BUFEND EQU    *
107 MAXLEN EQU    BUFEND-BUFFER

109 RDREC  CSECT
110 .
115 .      SUBROUTINE TO READ RECORD INTO BUFFER
120 .
122        EXTREF BUFFER, LENGTH, BUFEND
125        CLEAR  X
130        CLEAR  A
132        CLEAR  S
133        LDT    MAXLEN
135 RLOOP  TD     INPUT
140        JEQ    RLOOP
145        RD     INPUT
150        COMPR  A, S
155        JEQ    EXIT
160        +STCH  BUFFER, X
165        TIXR   T
170        JLT    RLOOP
175 EXIT   +STX   LENGTH
180        RSUB
185 INPUT  BYTE   X'F1'
190 MAXLEN WORD   BUFEND-BUFFER+3

193 WRREC  CSECT
195 .
200 .      SUBROUTINE TO WRITE RECORD FROM BUFFER
205 .
207        EXTREF LENGTH, BUFFER
210        CLEAR  X
212        +LDT   LENGTH
215 WLOOP  TD     =X'05'
220        JEQ    WLOOP
225        +LDCH  BUFFER, X
230        WD     =X'05'
235        TIXR   T
240        JLT    WLOOP
245        RSUB
255        END    FIRST

