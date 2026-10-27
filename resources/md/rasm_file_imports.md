# Importation de fichiers

## INCLUDE nom_de_fichier

```
INCLUDE 'monfichier.asm'
; IL N'Y A PLUS DE compatibilité du READ avec Winape !!!
```

La directive lit le fichier texte et l'intègre au code source à l'emplacement de l'instruction de lecture. Le chemin relatif de lecture a pour racine l'emplacement du fichier depuis lequel est faite l'inclusion. En utilisant un chemin absolu, on peut passer outre la référence à la racine du source. Si le fichier n'est pas trouvé, on le cherchera dans les éventuels répertoires d'inclusion déclarés avec la ligne de commande. Il n'y a pas de limite à la récursivité dans l'inclusion, faites attention à ce que vous faites.

Vous pouvez vous protéger de la réentrance de la façon suivante:

```
ifndef __SYMBOL__
__SYMBOL__ = 1

;; Code
endif
```

## INCBIN nom_de_fichier, options

INCBIN est capable d'importer des fichiers binaires de plusieurs façons.

Chaque paramètre est optionnel (sauf ceux fonctionnant par paires où la paire est requise à la suite)

```
INCBIN 'monfichier.bin'
INCBIN 'monfichier.bin',offset
INCBIN 'monfichier.bin',offset,longueur
```

L'ordre d'apparition des tags n'a pas d'importance. Par contre les valeurs numériques sont toujours interprétées dans l'ordre offset,longueur,offset étendu (l'offset étendu n'a d'utilité que pour du vieux code compatible Winape)

```
INCBIN 'monfichier.bin',EXISTS,offset,longueur
INCBIN 'monfichier.bin',offset,EXISTS,longueur
INCBIN 'monfichier.bin',offset,longueur,EXISTS
```

Les options suivantes peuvent être cumulées

```
INCBIN 'monfichier.bin',OFF ; lire les données en mémoire mais ne pas les considérer écrits pour pouvoir patcher les données lues
INCBIN 'monfichier.bin',EXISTS ; charger le fichier si et seulement si il existe
INCBIN 'monfichier.bin',SKIPHEADER ; sauter 128 octets si un entête AMSDOS est trouvé
INCBIN 'monfichier.bin',REVERT ; lire les données à l'envers
; les deux commandes ci-dessous ont le même résultat d'exécution
INCBIN 'monfichier.bin',REVERT,EXISTS,SKIPHEADER,offset,OFF,longueur
INCBIN 'monfichier.bin',OFF,REVERT,offset,EXISTS,longueur,SKIPHEADER
```

Les options suivantes sont mutuellement exclusives

```
INCBIN 'monfichier.bin',REMAP,numColumn
INCBIN 'monfichier.bin',VTILES,numLines
INCBIN 'monfichier.bin',ITILES,largeur
INCBIN 'monfichier.bin',GTILES,largeur
INCBIN 'monfichier.bin',SLICE,largeur,hauteur[,MISALIGNED]
INCBIN 'monfichier.bin',ISLICE,largeur,hauteur[,MISALIGNED]
```

INCBIN est capable d'importer des fichiers WAV, ils sont alors décodés selon leur format, avec un downmix en mone (une seule voie) et ensuite transformés selon l'option

```
INCBIN 'monfichier.wav',SMP
INCBIN 'monfichier.wav',SM2
INCBIN 'monfichier.wav',SM4
INCBIN 'monfichier.wav',DMA,preamp,options,options,...
```

Les données lues seront directement injectées dans le binaire de destination. Les paramètres optionnels sont compatibles avec la fonction INCBIN de Winape. Cependant, l'offset n'est pas limité à 64K comme avec Winape qui ne sait gérer que des valeurs 16 bits. L'offset étendu est là pour compatibilité, son usage est à éviter pour rester lisible.

- Il est possible de donner un offset négatif, qui sera alors relatif à la fin du fichier 
- Il est possible de donner une taille de fichier négative. La taille lue sera égale à la taille totale du fichier + cette valeur négative. Pour tout lire sauf les 10 derniers octets, on utilisera une taille de -10. 
- Une taille nulle chargera tout le fichier (vous en aurez besoin pour utiliser le paramètre optionnel OFF) 
- Le paramètre OFF permet de charger un fichier pour initialiser la mémoire. C'est à dire qu'il sera possible d'assembler ou produire du binaire par dessus car ce paramètre désactive le contrôle d'écrasement. 
- Le tag EXISTS ne charge le fichier que si il existe, sinon, aucune erreur n'est remontée 
- Le tag REVERT permet d'importer un fichier binaire dans le sens inverse de lecture 
- ITILES permet d'importer des tiles avec une hauteur multiple de 8, en entrelaçant les lignes dans l'ordre suivant: 0,1,3,2,6,7,5,4. De plus les octets sont encodés en zig zag. Le paramètre correspond à la largeur en octet des tiles. 
- GTILES fait la même chose que ITILES sans le zig-zag 
- REMAP calcule la hauteur des colonnes à partir de l'argument (nombre de colonnes), puis met les colonnes à plat (à partir d'une image 2D globale) 
- VTILES calcule la largeur d'une ligne à partir d'un nombre de lignes donné, puis mets à plat les données en colonnes 
- SLICE réorganise les tuiles en tranches. Le passage d'une tranche à l'autre se fait en incrémentant le poids fort 
- ISLICE réorganise les tuiles en tranches. Le passage d'une tranche à l'autre se fait en incrémentant le poids fort. Les tranches sont organisées 8 à 8 selon le code de Gray 
- Les fichiers WAV sont à priori tous supportés quel que soit leur format, mono-canal ou multi-canal. Une fusion des voix sera réalisée en cas de fichier multi-canal. La fréquence d'échantillonage n'est pas prise en compte. 
- Le format SMP contient une valeur (0-15) par octet 
- Le format SM2 contient deux valeurs par octet, le premier échantillon étant stocké dans le quartet de poids fort 
- Le format SM4 contient quatre valeurs de sample par octet, à raison de deux bits par octet, les valeurs 0,1,2 et 3 correspondant aux volumes 0,13,14 et 15. On peut ainsi coder toute l'amplitude audio de l'AY avec 2 bits. 
- Le format DMA va convertir chaque échantillon en une liste de commandes DMA spécifique aux Amstrad Plus. Le sample devra au préalable avoir été converti à 15600Hz. 
- Le format DMA peut avoir plusieurs options supplémentaires: DMA_INT pour déclencher une interruption à la fin du sample, DMA_CHANNEL_A, B ou C pour choisir le canal de l'AY par défaut (C) ainsi que DAM_REPEAT,repetition pour répéter le sample de 1 à 4095 fois. 

```
org #4000
incbin 'bank.raw',0,0,0,OFF ; charger le binaire en #4000 avec désactivation du contrôle d'écrasement
org #4000
defb #BB ; écraser le premier octet du fichier sans erreur
```

## INCBIN de fichiers multiples

Il est possible d'utiliser les tags de variables dans le nom des fichiers à charger, permettant le chargement en lots de séquences.

```
repeat 10,cpt
  incbin 'monfichier{cpt].bin'
rend
```

# Importation de fichiers avec compression à la volée

Rasm intègre de nombreuses méthodes de compression que l'on peut directement utiliser dans le code assembleur, soit en définissant des sections à compresser, soit en incluant des binaires qui seront compressés à la volée. Pour la décompression, on pourra utiliser les routines distribuées avec RASM (conseillé, certains crunchers évoluant avec le temps).

## Liste des équivalent à la commande INCBIN avec compression

- INCL48 même fonctionnement que la commande INCBIN avec compression LZ48 à la volée 
- INCL49 même fonctionnement que la commande INCBIN avec compression LZ49 à la volée 
- INCLZ4 même fonctionnement que la commande INCBIN avec compression LZ4 à la volée 
- INCZX0 même fonctionnement que la commande INCBIN avec compression ZX0 à la volée 
- INCZX0B même fonctionnement que la commande INCBIN avec compression ZX0 BACKWARD à la volée 
- INCZX7 même fonctionnement que la commande INCBIN avec compression ZX7 à la volée 
- INCEXO même fonctionnement que la commande INCBIN avec compression EXOMIZER à la volée 
- INCAPU même fonctionnement que la commande INCBIN avec compression AP-Ultra à la volée 
- INCLZSA1 même fonctionnement que la commande INCBIN avec compression LZSA1 à la volée 
- INCLZSA2 même fonctionnement que la commande INCBIN avec compression LZSA2 à la volée 

# Imports spéciaux

Il est possible de précharger les espaces mémoires de RASM avec un snapshot ou une cartouche

## SNAPINIT fichier_snapshot

## CPRINIT fichier_cartouche

# Imports dédiés aux tuiles

## INCBIN SLICE,largeur,hauteur[,MISALIGNED]

La directive importe un jeu de tuiles de dimension largeurxhauteur  

La taille du fichier importé doit être un multiple de largeurxhauteur  

Le nombre de lignes des tuiles doit être un multiple de 8 avec l'option ISLICE qui code en gray  

Si l'import n'est pas aligné, un warning est déclenché et l'import réalisé  

Vous pouvez forcer un import sans être aligné avec le tag additionnel MISALIGNED

L'ensemble des tuiles est réparti dans différents groupes. Le nombre de tuiles dans un groupe dépend de la largeur des tuiles. Toutes les premières lignes des tuiles sont logées dans le premier bloc de 256 octets. Avec des tuiles de 8 octets de large, vous aurez 32 tuiles par groupe. Un groupe occupe 256 octets x hauteur d'une tuile (moins quelques octets sur le dernier bloc de 256 octets si il contient moins de tuile que le maximum possible.

Intérêt : stocker un seul jeu de tuile sur lequel on peut appliquer des optimisations de ligne (incrément du poids faible) et des optimisations de colonne (incrément du poids fort)

