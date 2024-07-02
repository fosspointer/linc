[life.b -- John Horton Conway's Game of Life
(c) 2021 Daniel B. Cristofani
http://brainfuck.org/]

>>>->+>+++++>(++++++++++)[[>>>+<<<-]>+++++>+>>+[<<+>>>>>+<<<-]<-]>>>>[
  [>>>+>+<<<<-]+++>>+[<+>>>+>+<<<-]>>[>[[>>>+<<<-]<]<<++>+>>>>>>-]<-
]+++>+>[[-]<+<[>+++++++++++++++++<-]<+]>>[
  [+++++++++.-------->>>]+[-<<<]>>>[>>,----------[>]<]<<[
    <<<[
      >--[<->>+>-<<-]<[[>>>]+>-[+>>+>-]+[<<<]<-]>++>[<+>-]
      >[[>>>]+[<<<]>>>-]+[->>>]<-[++>]>[------<]>+++[<<<]>
    ]<
  ]>[
    -[+>>+>-]+>>+>>>+>[<<<]>->+>[
      >[->+>+++>>++[>>>]+++<<<++<<<++[>>>]>>>]<<<[>[>>>]+>>>]
      <<<<<<<[<<++<+[-<<<+]->++>>>++>>>++<<<<]<<<+[-<<<+]+>->>->>
    ]<<+<<+<<<+<<-[+<+<<-]+<+[
      ->+>[-<-<<[<<<]>[>>[>>>]<<+<[<<<]>-]]
      <[<[<[<<<]>+>>[>>>]<<-]<[<<<]]>>>->>>[>>>]+>
    ]>+[-<<[-]<]-[
      [>>>]<[<<[<<<]>>>>>+>[>>>]<-]>>>[>[>>>]<<<<+>[<<<]>>-]>
    ]<<<<<<[---<-----[-[-[<->>+++<+++++++[-]]]]<+<+]>
  ]>>
]

[This program simulates the Game of Life cellular automaton.

It duplicates the interface of the classic program at
http://www.linusakesson.net/programming/brainfuck/index.php,
but this program was written from scratch.

Type e.g. "be" to toggle the fifth cell in the second row, "q" to quit,
or a bare linefeed to advance one generation.

Grid wraps toroidally. Board size in parentheses in first line (2-166 work).

This program is licensed under a Creative Commons Attribution-ShareAlike 4.0
International License (http://creativecommons.org/licenses/by-sa/4.0/).]
