## Déclaration

```
MACRO nom,param1,param2,param3,...
```

Une macro est une façon d'étendre le langage en définissant un bloc d'instructions, délimité par MACRO / MEND (ou ENDM) et qui pourra être inséré ultérieurement dans la code par simple utilisation du nom de la macro. Les macros peuvent prendre des paramètres ce qui rend possible l'utilisation de conditions dans le code. À chaque appel, le code de la macro est inséré et les paramètres déclarés sont substitués. Ensuite le code est interprété de façon classique. Exemple d'une écriture indirecte longue distance:

```
macro LDIXREG register,dep
if {dep}<-128 || {dep}>127
   push bc,ix
   ld bc,{dep}
   add ix,bc
   ld (ix+0),{register}
   pop ix,bc
else
   ld (ix+{dep}),{register}
endif
mend
```

Pour que l'assembleur s'y retrouve plus facilement, la convention de passage des paramètres choisie est analogue à celle de Winape, avec utilisation des paramètres entre chevrons.

## Appel d'une macro

Attention! RASM ne peut pas savoir si vous voulez déclarer un label quand vous vous trompez dans l'écriture du nom d'une macro sans paramètre. Pour palier à ce défaut, vous pouvez ajouter un paramètre fictif "(void)" qui déclenchera une erreur si le nom de la macro n'est pas connu. De plus, si vous souhaitez forcer l'utilisation de cette syntaxe, il est possible de lancer RASM avec l'option -void . L'utilisation d'une macro sans paramètre provoquera une erreur.

```
macro withoutparam
nop
mend
withoutparam (void) ; appel sécurisé
```

## Macro avec nombre de paramètres variable

Si on termine une suite de paramètres avec trois points "…" l'assembleur autorisera un nombre infini de paramètres lors de l'appel

Au sein de la macro, on peut avoir l'information du nombre de paramètres envoyés avec le tag {#}

Chaque paramètre (même ceux situés avec les …) est accessible avec son numéro, en commençant à zéro => {3} pour le quatrième paramètre (0,1,2, 3!)

Il est possible de préciser une substitution par défaut au cas où l'appelant n'envoie pas assez de paramètre => {3|=0x44} si le paramètre n'est pas présent, remplacer par la valeur 0x44

Exemple

```
; do nothing unless extended param
macro zero,arg1,...
if {#}==1 : print 'no extended param' : endif
if {#}==2 : defb {1} : endif ; sort un DEFB si 2 paramètres sont envoyés à la macro
defb {2|=0x44} ; utilise le 3è paramètre ou 0x44 par défaut
mend
```

## Appel avec paramètres dynamiques

Il est possible d'évaluer les paramètres avant de les transmettre à la macro

```
macro test myarg
   defb {myarg}
mend

; équivalent à defb 1, 2
repeat 2
   test repeat_counter
rend

; équivalent à defb 1,1
repeat 2
   test {eval}repeat_counter ; le premier appel dans la boucle va évaluer le compter et claquer la valeur
rend
```

## Décomposition des registres 16 bits

Dans une macro, il est possible d'utiliser .low et .high pour utiliser le poids fort ou le poids faible d'un registre 16 bits. Exemple:

```
macro add16,R1,R2  

ld a,{R1].low  

add {R2}.low  

ld {R1}.low,a  

ld a,{R1].high  

adc {R2}.high  

ld {R1}.high,a  

mend

```
