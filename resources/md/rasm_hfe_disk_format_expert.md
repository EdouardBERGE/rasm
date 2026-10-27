
La commande HFE permet de créer et écrire dans un ou plusieurs fichiers HFE (pas de modification pour le moment).

## HFE INIT,'nomfichier.hfe'

## HFE CLOSE

## HFE SIDE,face

Permet de se positionner sur la face du HFE courant

## HFE TRACK,piste

Permet de se positionner sur la piste du HFE courant. La capacité de la disquette est réallouée au besoin.

## HFE ADD_TRACK_HEADER

Ajoute un entête de piste à la piste courante.

## HFE ADD_SECTOR,piste,face,ID,tailleSecteur,offsetMemoire (de la bank courante)

Ajoute un secteur à la piste (données en provenance de Rasm)

## HFE ADD_GAP,taille[,octet de GAP]

Ajoute un ou plusieurs octets sur la piste. La valeur par défaut est 0x4E, comme la GAP standard.

## HFE COMPLETE_TRACK

Rempli l'espace restant de la piste jusqu'à atteindre 6250 octets (plus propre).

# Byte Mode

## HFE START_CRC

En mode octet, on peut démarrer le calcul d'un CRC (pour l'écrire dans un entête ou après les données d'un secteur) qui intercepte l'écriture des octets.

## HFE ADD_BYTE,octet,…

Ajoute un octet à la piste courante. Pour ajouter des octets de synchro (0xC2 et 0xA1 supportés), il faut ajouter le tag SYNCHRO avec un OR (symbole pipe '|')

## HFE OUTPUT_CRC

Ajoute les deux octets du CRC courant sur la piste.

# Exemple Basique, création d'une disquette DATA

```
BANK
sector2 defs 512,0xE5

HFE INIT,'floppy.hfe'
HFE SIDE,0

curtrack=0
repeat 40
HFE TRACK,curtrack
HFE ADD_TRACK_HEADER
curid=#C1
repeat 9
        HFE ADD_SECTOR,curtrack,0,curid,2,sector2
        HFE ADD_GAP,#52
        curid+=1
rend
HFE COMPLETE_TRACK
curtrack+=1 
rend
```

# Exemple avancé

```
curtrack=0
HFE TRACK,curtrack 
HFE ADD_TRACK_HEADER
curid=#F0
repeat 16
        HFE ADD_BYTE,0,0,0,0,0,0,0,0,0,0,0,0 ; 12
        HFE START_CRC
        repeat 3 : HFE ADD_BYTE,0xA1 | SYNCHRO : rend
        HFE ADD_BYTE,0xFE
        HFE ADD_BYTE,curtrack
        HFE ADD_BYTE,0
        HFE ADD_BYTE,curid
        HFE ADD_BYTE,1
        HFE OUTPUT_CRC
        repeat 22 : HFE ADD_BYTE,0x4E : rend
        HFE ADD_BYTE,0,0,0,0,0,0,0,0,0,0,0,0 ; 12

        HFE START_CRC
        repeat 3 : HFE ADD_BYTE,0xA1 | SYNCHRO : rend
        HFE ADD_BYTE,0xFB

        repeat 256 : HFE ADD_BYTE,0xE5 : rend
        HFE OUTPUT_CRC

        HFE ADD_GAP,#10 ; small GAP
        curid+=1
rend
HFE COMPLETE_TRACK
```

