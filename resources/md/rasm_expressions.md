# Alias

Il est possible de créer un nombre illimité d'alias avec le mot clef EQU. Ces alias ne peuvent pas être modifiés une fois qu'ils sont définis, au contraire des variables. Les assembleurs classiques tendent à mélanger les notions de label, de variable et d'alias. Ici un alias ne changera pas de valeur au cour de l'assemblage, même en cas d'utilisation de sections compressées.

```
mon_alias EQU #4000
ld hl,mon_alias
```

De la même façon que l'usage d'un MODULE préfixe les labels, les alias sont aussi concernés. Il est aussi possible de déclarer des alias locaux en les préfixant par @

# Variables dynamiques

Il est possible de créer un nombre illimité de variables avec l'opérateur =. Ces variables doivent être initialisés avant usage (on ne fait pas du Basic!). Il est possible de les déclarer en même temps qu'une boucle de répétition (voir directive REPEAT). Les variables sont au format double flottant en interne. Comme pour toutes les expressions de RASM, un arrondi correct est réalisé en fin de chaine, lorsqu'on voudra le convertir en valeur 8 bits ou 16 bits.

Exemple de création d'une table de sinus typique:

```
ang=0
repeat 256
defb 127*sin(ang)
ang=ang+360/256
rend
```

# Variables dynamiques et noms générés

Dans le cadre de boucles (ou non), il est possible de générer des noms de variables. L'usage type pourrait être l'utilisation de tableaux à n dimensions.

Exemple:

```
repeat 256,x
mavariable{x}=x*5
rend
```

# Valeurs littérales

Format de la littérale selon son préfixe/suffixe :

- décimal : commence par un chiffre et ne contient que des chiffres  
- binaire : préfixe `%` ou `0b`, ou suffixe `b`  
- octal : préfixe `@`  
- hexadécimal : préfixe `#`, `$` ou `0x`, ou suffixe `h`  
- ASCII : un seul caractère entre quotes (`'x'` ou `"x"`), éventuellement converti par la directive `CHARSET` active  
- alias / variable / label : commence par une lettre, `.` (proximité) ou `@` (local)  
- `$` seul : adresse de début de l'instruction à venir*  
- `$$` : adresse physique de début de l'instruction à venir

*Lors d'une directive `DEFB`, `DEFW`, `DEFI`, `DEFR` ou `DEFS`, l'adresse courante est celle de l'élément en cours. Toutes ces directives produiront les mêmes données qu'elles soient utilisées avec plusieurs arguments ou plusieurs directives à la suite, au contraire de plusieurs assembleurs tels que Pasmo, AS80 et UZ80 pour ne citer qu'eux.

Utilisateurs de DAMS : le symbole `&` est réservé à l'opération `AND` ; utiliser si besoin l'option de ligne de commande `-amper`

# Caractères autorisés

Entre quotes, tous les caractères sont autorisés (avec l'option de ligne de commande -fq), à vos risques et périls concernant la conversion ASCII vers l'Amstrad. En dehors des quotes, vous pourrez utiliser toutes les lettres, tous les chiffres, le point, l'arobas, les parenthèses, le dollar, les opérateurs courants, dièse, paragraphes et chevrons, les deux types de quotes ainsi que les caractères échappés `\t`, `\n`, `\r`,  `\f`, `\v`, `\b`, `\0`.

En utilisant l'option -utf8 de la ligne de commande, les caractères suivants seront remplacés par leurs valeurs respectives dans les tables ASCII du Basic Amstrad Français ou Espagnol.

```
à = 64 ou @
ç = 92 ou
é = 123 ou {
ù = 124 ou |
è = 125 ou }
Ñ = 161
ñ = 171
¿ = 174
¡ = 175
```

# Opérateurs de calcul

RASM utilise un moteur d'expression à priorités multiples (comme le C), il supporte les opérations classiques de multiplication, addition, soustraction, division, ainsi que les opérateurs suivants:

- `\` : division entière (surtout utilisé pour les calculs d'adresse, préférez toujours hi(adresse) )
- `^` ou `XOR` : pour le OU exclusif
- `%` ou `MOD` : pour le modulo
- `&` ou `AND` : pour l'opérateur logique ET
- `&&` : pour l'opérateur booléen ET
- `|` ou `OR` : pour l'opérateur logique OU
- `||` : pour l'opérateur booléen OU
- `<<` : décalage à gauche
- `>>` : décalage à droite
- `hi(n)` : poids fort de l'entier 16 bits
- `lo(n)` : poids faible de l'entier 16 bits
- `sin(n)` : renvoie le sinus d'un angle en degrés
- `cos(n)` : renvoie le cosinus d'un angle en degrés
- `asin(n)` : calcul d'arc-sinuc
- `acos(n)` : calcul d'arc-cosinus
- `atan(n)` : calcul d'arc-tangente
- `int(n)` : conversion en entier
- `frac(n)` : récupère la fraction d'un nombre flottant
- `floor(n)` : conversion d'un double à l'entier inférieur
- `ceil(n)` : conversion d'un double à l'entier supérieur
- `abs(n)` : valeur absolue
- `rnd(n)` : nombre aléatoire entre 0 et n-1
- `ln(n)` : logarithme népérien
- `log10(n)` : logarithme base 10
- `exp(n)` : exponentielle
- `pow2(n)` : calcule 2 puissance N
- `sqrt(n)` : racine carrée
- `==` : égalité (= en mode MAXAM)
- `!=` ou `<>` : différent de
- `<=` : inférieur ou égal
- `>=` : supérieur ou égal
- `<` : inférieur
- `>` : supérieur
- `getnop('code assembleur')` : renvoie le temps d'exécution du code en NOP (Amstrad)
- `gettick('code assembleur')` : renvoie le temps d'exécution du code en Ticks (Spectrum)
- `duration('code assembleur')` : renvoie le temps d'exécution selon l'usage courant du source en cours
- `get_r(n)` : récupère la couleur rouge d'une encre Amstrad Plus
- `get_v(n)` : récupère la couleur verte d'une encre Amstrad Plus
- `get_b(n)` : récupère la couleur bleue d'une encre Amstrad Plus
- `set_r(n)` : converti un niveau de rouge (0 à 15) en sa couleur Amstrad Plus
- `set_v(n)` : converti un niveau de vert (0 à 15) en sa couleur Amstrad Plus
- `set_b(n)` : converti un niveau de bleu (0 à 15) en sa couleur Amstrad Plus
Note : les fonctions SET_<composante> peuvent être jumelées avec un OR pour combiner les trois composantes en une seule couleur Amstrad Plus
- `soft2hard_ink(n)` : conversion d'une encre Basic en encre Hardware Gate Array
- `hard2soft_ink(n)` : conversion d'une encre Hardware Gate Array en couleur Basic
- `filesize('nom de fichier')` : renvoie la taille en octet d'un fichier
- `getsize('code assembleur')` : donne la taille en octet d'une ou plusieurs instructions assembleur
- `is_register('chaine de caractère')` : renvoie 0 ou 1 selon que la chaine donnée soit un registre Z80
- `filebyte('fichier',offset)` : renvoie la valeur de l'octet lu dans le fichier à l'offset

## Opérateurs à plusieurs paramètres
- `pow(n,p)` : calcule n puissance p
- `fmod(n,m)` : calcule le modulo m de n
- `atan2(y,x)` : calcule l'arc tangente de y/x
- `hypot(x,y)` : calcule l'hypothénuse du triangle rectangle de côté x et y
- `ldexp(x,exp)` : calcule x * 2 puissance exp
- `fdim(x,y)` : renvoie la différence positive de x-y, résultat toujours positif ou nul
- `fstep(n,v)` : si n>=v alors renvoie 1 sinon 0
- `fmax(a,b)` : renvoie le maximum des deux valeurs
- `fmin(a,b)` : renvoie le minimum des deux valeurs
- `clamp(n,min,max)` : retourne la valeur n en la forçant dans l'intervale min:max
- `lerp(i1,i2,n)` : calcule i1+n*(i2-i1)
- `isgreater(v,seuil)` : si v est supérieur strict au seuil, renvoie 1 sinon 0
- `isless(v,seuil)` : si v est inférieur strict au seuil, renvoie 1 sinon 0
- `fremain(n,d)` : calcule le reste du quotient de n et d
- `countnops(label1,label2)` : calcule le nombre de nops représentatif du temps machine entre deux labels

RASM fait tous ses calculs internes en nombre flottant double précision. Un arrondi correct est réalisé en fin de chaine de calcul pour les besoins en nombres entiers. Si l'évaluation d'une expression aboutit à une erreur de calcul, le résultat de l'évaluation sera forcé à zéro.

## Priorité des opérateurs

| Opérateur | Prévalence Rasm | Prévalence Maxam |
|---|---|---|
| () | 0 | 0 |
| ! | 1 | 464 |
| * / % | 2 | 464 |
| + - | 3 | 464 |
| « » | 4 | 464 |
| < <= == => > != | 5 | 464 |
| & AND | 6 | 464 |
| \| OR | 7 | 464 |
| ^ XOR | 8 | 464 |
| && | 9 | 6128 |
| \| \| | 10 | 6128 |

