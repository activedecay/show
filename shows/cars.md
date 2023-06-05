. define-image pink pink-lambo
. define-image bluel blue-lambo
. define-image red red-chiron
. define-image neon neon-buggati
. define-image green green-bently
. define-image blue blue-chiron
. define-image grey grey-subi
. define-image car car

. # cars
. bg 0 0 0
. color 1 1 1
. y .4
. font .5 left
There are cars on every slide!
. image bluel 0 0 1 1 0 .01 .125 .125
. image pink 0 0 1 1 .125 .01 .125 .125
. image red 0 0 1 1 .25 .01 .125 .125
. image neon 0 0 1 1 .375 .01 .125 .125
. image green 0 0 1 1 .5 .01 .125 .125
. image blue 0 0 1 1 .625 .01 .125 .125
. image grey 0 0 1 1 .75 .01 .125 .125
. image car 0 0 1 1 .875 .01 .125 .125

# slide
. using cars

. # temp
. bg 1 1 1
. color 0 0 0
. y .9
strange!

# slide bluel
. using cars
. image bluel 0 0 1 1 .005 .135 .99 .865
. color .4 .4 1
. font .4 left
blue


# slide strange
. using temp
. font .1
strange behavior: templates define bg
which is saved across slides. the previous
slide was not supposed to have a white bg!

# slide pink
. using cars
. image pink 0 0 1 1 .005 .135 .99 .865
. color 1 .4 .8
pink!

# slide red
. using cars
. image red 0 0 1 1 .005 .135 .99 .865
. color 1 0 0
red!

# slide neon
. using cars
. image neon 0 0 1 1 .005 .135 .99 .865
. color 1 1 1
neon!

# slide green
. using cars
. image green 0 0 1 1 .005 .135 .99 .865
. color 0 1 0
green!

# slide bluec
. using cars
. image blue 0 0 1 1 .005 .135 .99 .865
. color 0 0 1
blue 2!

# slide grey
. color 1 1 1 .8
. using cars
. image grey 0 0 1 1 .005 .135 .99 .865
. y .5
grey!
 
# slide car 
. color 1 1 1 .5
. using cars
. image car 0 0 1 1 .005 .135 .99 .865
also gray!