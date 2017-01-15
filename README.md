## Documentation
### Introduction
This programming language is inspired by Pascal.
### Comments
Comments begin with `{`, and end with `}`.
### Lexical Elements

    # useful regular expressions
    letter = [a-zA-Z]
    digit = [0-9]

    IDENT           letter(letter|digit|_)*
    INTEGER         (digit)+
    PLUS            \+
    MINUS           -
    TIMES           \*
    DIVIDE          /
    EQ              =
    NEQ             <>
    LT              <
    GT              >
    LTE             <=
    GTE             >=
    LPAR            (
    RPAR            )
    LBRA            [
    RBRA            ]
    ASSIGN          :=
    COMMA           ,
    SEMICOLON       ;
    COLON           :
    DOT             .
    kwPROGRAM       program|PROGRAM
    kwFUNCTION      function|FUNCTION
    kwCONST         const|CONST
    kwVAR           var|VAR
    kwINTEGER       integer|INTERGER
    kwBEGIN         begin|BEGIN
    kwEND           end|END
    kwIF            if|IF
    kwTHEN          then|THEN
    kwELSE          else|ELSE
    kwARRAY         array|ARRAY
    kwOF            of|OF
    kwWHILE         while|WHILE
    kwDO            do|DO
    kwWRITE         write|WRITE
    kwREAD          read|READ
    EOF             EOF
    ERR             * #anything else
