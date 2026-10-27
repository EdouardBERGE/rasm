# Déclarations de données

## ENUM [nom,[valeur de départ,[incrément]]]

Permet de déclarer plus rapidement des alias dont les valeurs se suivent. Sans argument, aucun préfixe ne sera ajouté aux noms des alias. La valeur de départ est zéro par défaut et l'incrément par défaut est 1.

```
ENUM
zero
un
deux
MEND

assert zero==0
assert un==1
assert deux==2
```

On peut préciser un préfixe

```
ENUM prefix
zero
un
deux
MEND

assert prefix_zero==0
assert prefix_un==1
assert prefix_deux==2
```

On peut ajouter une valeur de départ.

```
ENUM prefix,5
zero
un
deux
MEND

assert prefix_zero==5
assert prefix_un==6
assert prefix_deux==7
```

Enfin, on peut ajouter l'incrément

```
ENUM prefix,5,5
zero
un
deux
MEND

assert prefix_zero==5
assert prefix_un==10
assert prefix_deux==15
```

Il est possible d'utiliser les tags de remplacement pour le préfixe

```
ENUM prefix{0},5,5
zero
un
deux
MEND

assert prefix0_zero==5
assert prefix0_un==10
assert prefix0_deux==15
```

Enfin, il est possible de changer la valeur courante lors de la déclaration des éléments

```
ENUM prefix
zero
un=5
deux
MEND

assert prefix_zero==0
assert prefix_un==5
assert prefix_deux==6
```

## CHARSET

```
CHARSET
CHARSET 'chaine',valeur
CHARSET debut,valeur
CHARSET debut,fin,valeur
CHARSET 'chaine','chaine'
```

- Chaine+Valeur, indexer chaque caractère de la chaine en partant de valeur et en incrémentant à chaque caractère 
- Début+Valeur, indexer chaque caractère à partir du code ASCII Début en partant de valeur et en incrémentant à chaque caractère 
- Idem mais de début à fin uniquement 
- Chaine+Chaine, pour établir des correspondances plus facilement (utilisation typique en chiffrement simple) 
- Aucun paramètre, réinitialiser tous les codes ASCII à leur valeur initiale 

## UTF8REMAP 'caractère UTF8','code 8 bits'

Certains caractères UTF8, en particulier les accents, les majuscules accentuées, certains symboles ne sont pas standards. Il est possible d'effecturer un mapping (à la manière de CHARSET) en les déclarant avec cette directive.

Notez qu'il faudra impérativement utiliser l'option de ligne de commande -fq pour que le préprocesseur autorise les caractères spéciaux.

```
utf8remap 'é',50
defb 'é' ; produira l'octet 50
```

## DB, DEFB, DM, DEFM valeur1, valeur2, valeur3, …

Cette directive prend 1 ou plusieurs paramètres et génère une suite d'octets représentative des paramètres en entrée. Les valeurs peuvent être littérales, une formule, un caractère ou même une chaine de caractères. On utilisera indifféremment DB, DM, DEFB, DEFM. Le code suivant produira la chaine 'Roudoudou'

```
org #7500
label DEFB 'r'-'a'+'A','oudoud',#6F,hi(label)
```

```
DEFB 'roudoudou\xFF' ; add hex value #FF at the end of the string
```

## STR chaine1, chaine2, …

Cette directive est similaire à DEFB sauf que le dernier caractère produit subit un OR #80 (mis à 1 du bit le plus significatif), surtout utilisée pour gagner 1 octet en déclarant des chaines de caractères. Les deux lignes de code suivantes font exactement la même chose

```
str 'roudoudou'
defb 'roudoudo','u'|#80
```

Il est possible de déclarer plusieurs chaines à la suite de la directive, chacune aura son marqueur de fin

```
str 'roudoudou','roudoudou'
defb 'roudoudo','u'|#80,'roudoudo','u'|#80
```

STR supporte les valeurs hexa de type \xFF à l'intérieur des chaines

Il est possible de mélanger des chaines et des expressions. Dans ce cas, le marqueur de fin ne sera pas ajouté à la chaine précédent l'expression

```
str 'roudou',0+'d','ou'
defb 'roudoudo','u'|#80
```

## DEFW valeur1,valeur2

Déclaration de valeurs 16 bits, il n'est plus possible d'utiliser des chaines de caractères, mais un caractère unique entre quotes reste toujours possible!

## DEFI valeur1,valeur2

Déclaration de valeurs 32 bits, il n'est plus possible d'utiliser des chaines de caractères, mais un caractère unique entre quotes reste toujours possible!

## DEFR valeur1,valeur2

Déclaration de valeurs réelles 40 bits du locomotive Basic, il n'est plus possible d'utiliser des chaines de caractères, mais un caractère unique entre quotes reste toujours possible!

## DEFF valeur1,valeur2

Déclaration de valeurs réelles 40 bits Microsoft IEEE-754, il n'est plus possible d'utiliser des chaines de caractères, mais un caractère unique entre quotes reste toujours possible!

## DF24 valeur1, …

Déclaration de valeurs réelles 24 bits suivant un stripdown 24 bits du FLOAT32. il n'est plus possible d'utiliser des chaines de caractères, mais un caractère unique entre quotes reste toujours possible! Utilisable avec la librairie assembleur du répertoire assets

# Déclarations et utilisation de structures

## STRUCT nom_du_prototype, nom_de_la_variable / ENDSTRUCT

Quand STRUCT est utilisée avec 1 paramètre (le nom), les labels qui suivent serviront à déclarer la structure, en particulier ses indexes  

Quand STRUCT est utilisée avec 2 paramètre (nom_type+nom_struct), une structure est créée de type nom_type et s'appellera nom_struct

Bien que RASM permette de mettre un peu n'importe quoi à l'intérieur de la déclaration d'une structure, l'usage pratique est de déclarer des labels et via les instructions de définition de données, une taille de champ. Il n'est pas nécessaire de renseigner une valeur aux instructions DEF

```
struct st1
  ch1 defw
  ch2 defb
endstruct

struct metast1
  struct st1 pr1
  struct st1 pr2
endstruct

struct metast1 bigst ; instanciation de metast1 en bigst

ld hl,bigst.pr2.ch1 ; récupération de l'adresse absolue d'un membre de bigst
ld a,(hl)

ld a,(ix+metast1.pr2.ch1 ; utilisation de l'offset du champ via la définition de la structure (pas besoin d'instancier pour cet usage)
```

## préfixe {SIZEOF}

Le préfixe {sizeof} devant un nom de structure, de sous-structure ou devant le champ d'une structure, permet de récupérer sa taille

ld a,{sizeof}metast1 ; LD A,6  

ld a,{sizeof}metast1.pr2 ; LD A,3  

ld a,{sizeof}metast1.pr2.ch1 ; LD A,2

## tableau de STRUCT

Il est possible d'instancier plusieurs fois la même structure, comme s'il s'agissait d'un tableau de structures, de la façon suivante

```
STRUCT mystruct my_instance,10
```

Cette façon de faire est différente d'un DEFS 10*{sizeof}mystruct car les données sont initialisées avec les valeurs par défaut si elles ont été définies lors de la déclaration de la structure, à la différence d'un remplissage à zéro. De plus il est possible d'adresse chacun des structures via un index en ajoutant l'index à la fin du nom de l'instance

```
ld hl,my_instances5 ; récupérer l'adresse de la cinquième structure
```

## déclaration de STRUCT et initialisation

```
STRUCT mystruct
field1 defs 10
ENDSTRUCT

STRUCT mystruct my_instance,1,'roudoudou' ; déclare 1 struct et rempli le premier champ avec 'roudoudou', qui sera complété par un zéro car la déclaration était vide
```

