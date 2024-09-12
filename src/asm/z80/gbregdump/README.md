# format for reg dump player

read byte.
if it is between 00 and 7F, write to register.
otherwise, special command:

val | description
----|-----------------------
 80 |
 EE | 
 EF | wait
 Fx | preset wait (from F0 to FF)
