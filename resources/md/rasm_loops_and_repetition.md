# Directives de boucles ou répétitions

## REPEAT, UNTIL, REND

La base!

```
REPEAT nb_repetition,variable_compteur,valeur_depart,valeur_pas
```

Les paramètres "variable compteur", "valeur départ" et "valeur du pas" sont optionnelles

La directive permet de répeter un bloc d'instructions. On peut soit fixer un nombre de répétitions, soit utiliser le mode conditionnel avec la directive de fin de bloc UNTIL. Pour compatibilité avec Vasm, il est possible de finir un bloc avec ENDREP ou ENDREPEAT plutôt que REND

```
repeat 10 : ldi : rend ; produire 10x LDI
```

```
repeat 10,x : ld a,x : rend ; produire LD A,1 : LD A,2 : LD A,3 ... LD A,10
```

```
cpt=90 : repeat : defb cpt : cpt=cpt-2 : until cpt>0 ; produire la suite 90,88,86, ... 2, 0
```

```
repeat 5,x : print x : rend ; affiche 1,2,3,4,5
repeat 5,x,0 : print x : rend ; affiche 0,1,2,3,4
repeat 5,x,0,2 : print x : rend ; affiche 0,2,4,6,8
```

## STARTINGINDEX [valeur[,incrément]]

Par défaut, les boucles REPEAT démarre leur compteur à 1. Il est possible de changer ce comportement pour tous les REPEAT suivants en spécifiant une valeur à cette directive: Typiquement zéro, mais vous pouvez spécifier n'importe quelle valeur.  

Le deuxième paramètre optionnel est la valeur de l'incrément (1 par défaut)  

Sans paramètre l'incrément et la valeur de départ sont réinitialisées à 1

```
repeat 5,x : print x : rend ; affiche 1,2,3,4,5
startingindex 0
repeat 5,x : print x : rend ; affiche 0,1,2,3,4
```

## WHILE, WEND

```
WHILE condition : WEND

WHILE condition,variable : WEND ; use variable if you need a counter starting from "startingindex"
```

Répète un bloc tant que la condition déclarée est vraie. À tout moment (à l'intérieur de la boucle) il est possible d'utiliser la variable interne WHILE_COUNTER pour récupérer le compteur de boucle.

```
cpt=5
while cpt>0
  ldi
  cpt=cpt-1
  print 'cpt=',cpt,' while_counter=',while_counter
wend
```

Va produire la sortie suivante sur la console

```
cpt= 4.00 while_counter= 1.00
cpt= 3.00 while_counter= 2.00
cpt= 2.00 while_counter= 3.00
cpt= 1.00 while_counter= 4.00
cpt= 0.00 while_counter= 5.00
```

## REPEAT_COUNTER, WHILE_COUNTER

Ces mots clefs sont à utiliser comme une variable à l'intérieur d'une boucle dont on souhaite récupérer le compteur. Usage peu pratique à part dans un WHILE, préférer déclarer une variable lors du REPEAT, croyez moi ;)

