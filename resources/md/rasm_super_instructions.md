# Syntaxes spéciales de RASM (super-instructions)

Raccourcis de macro-instructions : forme contractée => forme développée.

- `PUSH BC,DE,HL` => `PUSH BC:PUSH DE:PUSH HL`

- `POP BC,DE,HL` => `POP BC:PUSH DE:PUSH HL`

- `NOP 5` => `NOP:NOP:NOP:NOP:NOP`

- `LD BC,BC` => `LD B,B:LD C,C`
- `LD BC,DE` => `LD B,D:LD C,E`
- `LD BC,HL` => `LD B,H:LD C,L`
- `LD DE,BC` => `LD D,B:LD E,C`
- `LD DE,DE` => `LD D,D:LD E,E`
- `LD DE,HL` => `LD D,H:LD E,L`
- `LD HL,BC` => `LD H,B:LD L,C`
- `LD HL,DE` => `LD H,D:LD L,E`
- `LD HL,HL` => `LD H,H:LD L,L`
- `LD HL,(IX+n)` => `LD H,(IX+n+1):LD L,(IX+n)` ; idem avec BC et DE
- `LD (IX+n),HL` => `LD (IX+n+1),H:LD (IX+n),L` ; idem avec BC et DE

- `LD HL,SP` => `LD HL,0 : ADD HL,SP` ; attention, modifie les flags!

- `EXA` => `EX AF,AF'`

- `SRL BC` => `SRL B : RR C`
- `SRL DE` => `SRL D : RR E`
- `SRL HL` => `SRL H : RR L`

- `SRA BC` => `SRA B : RR C`
- `SRA DE` => `SRA D : RR E`
- `SRA HL` => `SRA H : RR L`

- `SLL BC` => `SLL C : RL B`
- `SLL DE` => `SLL E : RL D`
- `SLL HL` => `SLL L : RL H`

- `SLA BC` => `SLA C : RL B`
- `SLA DE` => `SLA E : RL D`
- `SLA HL` => `SLA L : RL H`

- `RR BC` => `RR B : RR C`
- `RR DE` => `RR D : RR E`
- `RR HL` => `RR H : RR L`

- `RL BC` => `RL C : RL B`
- `RL DE` => `RL E : RL D`
- `RL HL` => `RL L : RL H`

- `RLC BC` => `SLA B : RL C : RR B : RLC B`
- `RLC DE` => `SLA D : RL E : RR D : RLC D`
- `RLC HL` => `SLA H : RL L : RR H : RLC H`

- `RRC BC` => `SRL B : RR C : RL B : RRC B`
- `RRC DE` => `SRL D : RR E : RL D : RRC D`
- `RRC HL` => `SRL H : RR L : RL H : RRC H`

- `SRL8 BC` => `LD C,B : LD B,0`
- `SRL8 DE` => `LD E,D : LD D,0`
- `SRL8 HL` => `LD L,H : LD H,0`
- `SRL8 IX` => `LD XL,XH : LD XH,0`
- `SRL8 IY` => `LD YL,YH : LD YH,0`

- `RST Z,#38` => `JR Z,$+1` ; l'opcode est l'adresse relative de saut!
- `RST NZ,#38` => `JR NZ,$+1`
- `RST C,#38` => `JR C,$+1`
- `RST NC,#38` => `JR NC,$+1`

