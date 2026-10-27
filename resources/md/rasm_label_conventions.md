# Différents types de labels

RASM gère plusieurs types de labels en fonction de différents contextes

- Les labels globaux, accessibles avec leur nom depuis l'ensemble des sources 
- Les labels de proximité, accessibles rapidement par leur nom court, ou depuis n'importe quel endroit des sources par leur nom complet 
- Les labels locaux, accessibles uniquement depuis le périmètre d'une boucle ou d'une macro 
- Les modules, sorte de supers labels qui encapsulent des labels globaux (du coup, pas si globaux que ça, même si une fois de plus, ils restent accessibles depuis n'importe quel endroit des sources!) 

## Labels globaux

Simples, les labels globaux commençent systématiquement par une lettre ou un  

Ils sont accessibles de partout avec le même nom, ce sont les labels usuels que tout assembleur connait

```
BANK 0
monlabel1 jp monlabel2
BANK 1
monlabel2 jp monlabel1
```

## Labels de proximité

Les labels de proximité commençent par un point (".")  

En interne, un label global est créé, concaténant le dernier label global avec celui de proximité

```
BANK 0
label_global ld b,5
.label_proximite djnz .label_proximite
BANK 1
label_global2 ld b,5
jp label_global1.label_proximite
```

## Labels anonymes directionnels de proximité

Ce type de label est inspiré de spasm-ng (assembleur pour calculatrice TI)

```
_
 nop
 djnz _-
 jr _+
 nop
_
```

Le _ définit un point d'ancrage. Les instructions de saut (et uniquement elles) peuvent se référer au _ suivant ou précédent en utilisant + ou - comme dans l'exemple

## Labels locaux

À l'intérieur d'une boucle ou d'une macro, un label local est uniquement "vu" dans le périmètre de la boucle et uniquement cette boucle. Ainsi, on pourra imbriquer 3 boucles et utiliser le même nom de label local, ce sera en fait 3 labels différents en interne, chacun appartenant à sa boucle respective. Les labels locaux commençent par le symbole @

```
repeat 10
  @debut
  repeat 10
    ld b,5
    @debut djnz @debut
  rend
  djnz @debut ; réfère au premier @debut
rend
```

## Modules

Avec la directive MODULE, on déclare une sorte de super label qui préfixera tous les labels globaux, et par conséquent, tous les labels de proximité. L'utilisation de module rend caduque la règle de globalité des labels globaux ainsi que des alias! ^_^

On peut toujours appeler n'importe quel label de n'importe où, mais en respectant la hierarchie des modules

Pour ceux que le _ séparant le nom du module du label global perturbe, il est possible de redéfinir ce caractère par l'option de ligne de commande -msep

```
module soft1
label_global ; un label global, pas si global car encapsulé par le module soft1 !
djnz label_global
jp soft2_label_global

module soft2
label_global ; bis repetita
djnz label_global
jp soft1_label_global
```

Il est possible de générer des séries de modules à partir de variables

```
repeat 10,x
bank
module truc{x}
oneTwo nop
rend
```

## Export des labels

Depuis la version 2.1 rasm exporte des chunks et fichiers de symboles spéciaux pour fonctionner avec l'émulateur ACE  

Les labels exportés sont positionnés de façon précise dans l'émulateur (en RAM si déclarés en RAM, en ROM si déclarés en ROM)  

Néanmoins, on peut avoir besoin de reloger du code (obligatoirement en RAM) depuis une ROM  

Il est donc possible de déclarer des labels comme locaux (global par défaut) quand ils sont relogés afin de les voir dans l'émulateur

```
LABEL LOCAL ; tous les labels suivants seront considérés à leur emplacement de compilation
LABEL GLOBAL ; tous les labels suivants seront considérés dans l'espace adressable des 64K
```

Quand on assemble des fichiers simples ou à partir d'espaces temporaires (usage de BANK sans spécificité mémoire), on peut attribuer à l'espace temporaire courant une localisation pour l'émulateur

```
LOCALISATION RAM,4 ; tous les labels de l'espace seront attribués à la page 4 de la RAM
LOCALISATION ROM,LOWER ; tous les labels de l'espace seront attribués à la Lower ROM
LOCALISATION ROM,14 ; tous les labels de l'espace seront attribués à la ROM 14
```

