# Modes mémoires de RASM

Par défaut, RASM ouvre un espace mémoire temporaire de 64K. Vous pouvez écrire dedans, en fin d'assemblage, un fichier binaire sera produit partant du premier octet écrit jusqu'au dernier!

Si vous ouvrez un espace mémoire spécifique avec la directive BANK, alors RASM basculera en mode cartouche

## Bonnes pratiques et choix du mode mémoire

Précisez toujours le type de travail que vous souhaitez réaliser, il existe quelques modes de configuration et vérification de la mémoire.

## BUILDCPR <options>

Bascule RASM en mode cartouche, les BANK 0 à 31 sont accessibles, pour outrepasser la limitation voir la commande de compilation -xpr

Exemple pour créer une cartouche à partir de fichiers binaires

```
BUILDCPR
BANK 0 : INCBIN 'lower.bin'
BANK 1 : INCBIN 'lasuivante.bin'
; jusqu'à 31 :)
```

options :

EXTENDED => dépasser la limitation des 512k  

SYMBOLS => ajoute dans la cartouche des informations de debug pour l'émulateur ACE-DL  

"filename" => spécifier le nom de la cartouche en sortie

## BUILDTAPE <option>

Bascule RASM en mode K7, le binaire produit dans la BANK par défaut sera exporté au format CDT

option : "filename" => spécifier le nom de fichier K7 en sortie

## BUILDSNA <options>

Bascule RASM en mode snapshot, l'utilisation de la mémoire peut se faire soit par BANK de 16K numérotés de 0 à 259 (mais toujours vues comme des espaces indépendants de 64K par commodité), soit par BANKSET de 64K (numérotés de 0 à 64)

options :  

- V2 => permet de générer des snapshots plus simples que la carte M4 saura lire, les snapshots v3 étant plutôt réservés aux émulateurs pour le moment.  

- CPR => permet de générer une cartouche à partir des données ROMBANK 128 à 128+31  

- FORCE => forcer l'écriture du snapshot quand RASM est utilisé en version intégrée  

- "filename" => spécifier le nom du snapshot en sortie

## BUILDZX

Bascule RASM en mode snapshot mémoire pour le ZX, les banques accessibles vont de 0 à 7

Vous devez utiliser la directive RUN avec deux paramètres pour spécifier l'adresse de départ et l'emplacement de la pile!

```
BUILDZX
BANK 0
RUN #8000,#4000 ; entry-point #8000, stack location #4000
```

## BUILDROM concat

Bascule RASM en mode création de ROM, l'option concat permet un export concaténé des ROM en un seul fichier (plus simple pour gérer de gros projets si l'émulateur est assez malin pour comprendre qu'il faut s'étaler dans les ROM suivantes)

## BUILDOBJ

Bascule RASM en mode création d'objets binaires manipulables par un éditeur de liens (prototype)

# Directives de compilation mémoire

## NOCODE

Tout le code écrit après cette directive n'est pas écrit en mémoire mais les adresses sont calculées et l'adresse courante continue d'avancer

## CODE [SKIP]

Par défaut tout nouvel espace mémoire est ouvert en écriture, cette directive sert uniquement à sortir du mode NOCODE

Sans paramètre, on revient au mode normal à l'adresse courante

Avec le paramètre SKIP, on revient au mode normal, à l'adresse courante correspondant au moment où NOCODE était ouvert

## RUN adresse, ga_config

La directive RUN permet à RASM de modifier la valeur de PC et des connexions ROM lors d'un export snapshot, mais aussi de spécifier dans un export binaire AMSDOS ou sur DSK, l'adresse de démarrage du binaire

## ORG adresse_logique, adresse_physique

La directive ORG permet d'indique l'adresse de départ du code et des données à assembler. RASM permet l'utilisation de plusieurs ORG au sein d'un même espace mémoire, mais il contrôle toujours que les différentes zones ne se chevauchent pas. L'adresse logique est l'adresse utilisée pour les calculs de saut tandis que l'adresse physique est l'adresse réelle d'écriture du binaire produit.

```
org #8000
ld hl,$ ; code écrit en #8000 => LD HL,#8000
```

```
org #100
nop
org #8000,$
ld hl,$ ; code écrit en #101 mais avec l'adresse logique #8000 => LD HL,#8000
```

L'utilisation de l'adresse physique désynchronise l'adresse logique de l'adresse phyique, pour revenir à l'adresse physique, il faut écrire:

```
org #100
nop
org #8000,$
ld hl,$ ; LD HL,#8000
org $
ld hl,$ ; LD HL,#104
```

## ALIGN limite,remplissage

Si l'adresse d'écriture du code en cours n'est pas un multiple de la valeur d'alignement, on augmente l'adresse en conséquence. Par défaut, cette instruction ne produit pas d'octet sur la sortie. Cependant, les espaces de travail sont initialisés à zéro. Il est possible de préciser une valeur de remplissage avec le second paramètre.

```
org #8001
align 2 ; aligner le code sur l'adresse paire suivante, ici #8002
align 256,#55 ; aligner le code sur l'adresse multiple de 256 suivante, ici #8100, et remplir l'intervale avec #55
```

## CONFINE valeur

Cette directive permet de s'assurer que l'espace qui suit permet d'être adressé sans changement de poids fort. Elle permet d'optimiser le placement de petits tableaux de façon plus efficace qu'un alignement mémoire. Si un alignement est effectué, la directive produit un avertissement. La valeur du paramètre est de fait comprise entre 1 et 256  

En cas d'ajustement, un warning est affiché ainsi que le nombre d'octets perdus dans l'opération, permettant d'un coup d'oeil de décider si ça vaut le coup ou pas de mettre la zone ailleurs.

## LIMIT limite

Cette directive permet d'imposer une limite d'assemblage plus basse que celle des 65536 octets. Pour protéger une zone mémoire, on préfèrera utiliser la directive PROTECT

## PROTECT adresse_depart, adresse_fin

Cette directive permet d'empêcher l'écriture des données dans la zone délimitée par les paramètres début et fin de l'espace mémoire courant. Il est possible de définir autant de zones d'exclusion que l'on souhaite pour un même espace mémoire. Ces espaces ne peuvent pas se chevaucher. Si des données y sont écrites lors de l'assemblage, une erreur sera générée.

## SUMMEM adresse_depart, adresse_fin

Cette directive calcule la somme des octets compris entre adresse_depart et adresse_fin et écrit le résultat à l'endroit où se situe la directive. Le calcul est fait sur la banque courante et le résultat produit 1 octet.

## XORMEM adresse_depart, adresse_fin

Cette directive est similaire à SUMMEM mais applique l'opérateur XOR sur les octets compris entre adresse_depart et adresse_fin et écrit le résultat à l'endroit où se situe la directive. Le calcul est fait sur la banque courante et le résultat produit 1 octet.

```
checkrom
xor a ; initialiser le XORMEM
ld hl,0
ld bc,#1000
.computexor
xor (hl)
inc hl
ld d,a
dec bc
ld a,b
or c
ld a,d
jr nz,.computexor
ld hl,checksum
cp (hl)
jr nz,rom_KO
jr rom_OK
checkum xormem 0,#1000
```

## CIPHERMEM depart,fin,methode,clef

Fonction de chiffrement des données

- La première méthode s'initialise sur le premier octet puis XOR toutes les valeurs de l'intervale. 
- La deuxième méthode s'initialise sur le poids faible de l'adresse courante, ceci permet que le premier octet ne soit pas en clair, exception faite d'une adresse de départ multiple de 256! 
- La troisième méthode fait un XOR de chaque valeur avec le poids faible de leur adresse respective 

# Gestion des banques

RASM permet de travailler dans autant d'espaces mémoire que l'on souhaite. Pour ouvrir un espace mémoire de 64K (il y en a toujours un d'ouvert par défaut), on utilisera la directive BANK sans paramètre. Pour associer un espace mémoire à des ROM ou de la RAM, on pourra utiliser n'importe lequel des 256 slots disponibles, et même 260 en mode snapshot (64K+4M)

## BANK numero

La sélection d'un numéro de banque va positionner RASM en mode cartouche par défaut. Si vous voulez accéder aux banques d'un snapshot ou des ROM, il faut au préalable déclarer votre intention avec les commandes BUILDSNA ou BUILDROM. Le mode cartouche est de base limité à 32 ROM. Si vous dépassez la ROM 31, RASM va basculer automatiquement en mode cartouche étendue de type XPR (expérimental et uniquement supporté sous le manteau ;)

L'usage de la directive sans paramètre va ouvrir un espace temporaire de 64K qui sera "perdu" à moins que vous utilisiez la directive SAVE

AVERTISSEMENT: La sélection d'une BANK ouvre toujours un espace de 64K alors que seuls 16K seront enregistrés en mode ROM, cartouche ou snapshot. Afin de permettre de compiler à n'importe quelle adresse, le départ de ces 16K sera le premier octet écrit. Pour cette raison, il est recommandé de lire le log de sortie (adresse de départ, taille) pour bien valider que vous n'êtes pas décalé. La recommandation serait de poker un NOP sur le début si vous avez besoin de démarrer votre code ou vos data plus loin que le départ logique de la ROM/RAM.

## REBANK

Cette directive permet de revenir à la banque précédente (qu'elle soit figée ou temporaire). Il n'y a pas d'historique, seule l'avant-dernière banque utilisée est connue.

## BANKSET numero

L'usage de cette directive permet d'accéder (en mode de compilation snapshot avec la directive BUILDSNA) aux banks mémoires de façon groupée (par 64K). L'espace d'adressage est alors absolu tandis que l'usage de bank simple bascule RASM en mode relatif.

## ROMBANK numero|Lower|Next

En mode de compilation de snapshot (directive BUILDSNA), les BANK/BANKSET permettent d'adresser de la mémoire jusqu'à 4160k  

Il est possible de compiler (en plus de la RAM) dans des ROM grâce à la directive ROMBANK qui permet à RASM de discriminer la RAM et la ROM  

De nouveaux chunks seront ajoutés au fichier SNA, voyez avec les développeurs de votre émulateur préféré pour le support de ces chunks!

voir super snapshots

```
ROMBANK Next ;  Selectionne la ROM située juste après la ROM courante
```

```
ROMBANK n ;  avec n valeur de 0 à 255 permet de compiler dans l'une des 256 ROM standards du CPC
```

```
ROMBANK lower ; compiler pour la ROM basse
```

## préfixe {BANK}

À tout moment, vous pouvez préfixer un label avec le tag {BANK} qui vous renverra le numéro de ROM, pratique pour faire des bascules génériques.

```
BANK 0
ld a,{bank}hello ; LD A,1 car le label hello est en BANK 1
BANK 1
hello nop
```

## préfixe {PAGE}

À tout moment, vous pouvre préfixer un label avec le tag {PAGE}. Au lieu de renvoyer la valeur du label, vous aurez la valeur 16 bits à envoyer au Gate Array pour connecter la bonne page mémoire

```
BANK 0
ld bc,{page}hello ; LD BC,#7FC5
BANK 5
hello nop
```

## préfixe {PAGESET}

Similaire au préfixe {PAGE}, le préfixe {PAGESET} renvoie la valeur à donner au Gate Array pour connecter les 64K du BANKSET correspondant à la BANK

Ainsi, une demande de PAGESET sur un label situé dans l'extension 64K d'un 6128 renverra #7FC2

# Options spécifiques aux snapshots

## SETCPC modele

Choisir le modèle de CPC pour le snapshot, les valeurs autorisées sont

- 0 : CPC 464 
- 1 : CPC 664 
- 2 : CPC 6128 
- 4 : 6128 Plus 
- 5 : 464 Plus 
- 6 : GX-4000 

## SETCRTC modele

Choisir le modèle de CRTC pour le snapshot. Les valeurs autorisées vont de 0 à 4. Pour rappel, les CPC ont des CRTC 0,1,2 ou 4 et les Plus ou GX-4000 ont tous le CRTC 3

## SNASET param

```
SNASET registre, valeur ; usage à 2 paramètres
SNASET CRTC_REG, index, valeur ; usage à 3 paramètres
```

Différents paramètres du snapshot sont accessibles en écriture avec 2 paramètre, le regitre et la valeur:  

Z80_AF, Z80_F, Z80_A, Z80_BC, Z80_B, Z80_C, Z80_DE, Z80_E, Z80_D, Z80_HL, Z80_H, Z80_L, Z80_I, Z80_R, Z80_IFF0, Z80_IFF1, Z80_IX, Z80_IY  

Z80_IXL, Z80_IXH, Z80_IYL, Z80_IYH, Z80_SP, Z80_PC  

Z80_AFX, Z80_FX, Z80_AX, Z80_BCX, Z80_BX, Z80_CX, Z80_DEX, Z80_EX, Z80_DX, Z80_HLX, Z80_HX, Z80_LX  

GA_PEN, GA_ROMCFG,GA_RAMCFG,CRTC_SEL,ROM_UP,PPI_A,PPI_B,PPI_C,PPI_CTL,PSG_SEL  

CPC_TYPE,INT_NUM,FDD_MOTOR,FDD_TRACK,CRTC_TYPE,CRTC_HCC,CRTC_CLC,CRTC_RLC,CRTC_VAC  

CRTC_HSWC,CRTC_VSWC,CRTC_STATE,GA_VSC,GA_ISC,INT_REQ

Les registres du Gate Array, du CRTC et de l'AY sont accessibles en utilisant 3 paramètres: Le composant, l'index du registre et la valeur  

GA_PAL, CRTC_REG, PSG_REG

Référez-vous à la documentation du format snapshot pour vous y retrouver ;)

