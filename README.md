# User manual

How to compile and run (tested only on Windows Linux Subsystem running Ubuntu 20.04
and a Ubuntu 20.04 VM):

```
make outstation
make master
./outstation    # start this first in one terminal
./master        # start in another terminal while capturing packets
                # in your loopback adapter in wireshark
```

This is what happens during execution:
1. When `./outstation [local_address]` is executed, the outstation starts and listens on `127.0.0.1:20000 (TCP)`. The `local_address` is optional, and if not given will default to `70`;
2. After `./master [local_address remote_address]` is executed, it connects to the `127.0.0.1:20000` socket and sends a DNP3 read request with function code 01 from `local_address` to `remote_address`.
    - The address configuration is optional and defaults to `64` and `70` respectively.
    - The master program does not validate addresses and can be used for testing invalid addresses
    - The outstation program validates source and destination addresses. It should not be possible to use invalid addresses in this program.
3. The following settings illustrate the read request sent.

```
    Distributed Network Protocol 3.0
        Data Link Layer, Len: 11, From: 64, To: 70, DIR, PRM, Unconfirmed User Data
            Start Bytes: 0x0564
            Length: 11
            Control: 0xc4 (DIR, PRM, Unconfirmed User Data)
            Destination: 70
            Source: 64
            Data Link Header checksum: 0xfea3 [correct]
            [Data Link Header Checksum Status: Good]
        Transport Control: 0xc0, Final, First(FIR, FIN, Sequence 0)
            1... .... = Final: Set
            .1.. .... = First: Set
            ..00 0000 = Sequence: 0
        Data Chunks
        [1 DNP 3.0 AL Fragment (5 bytes): #6884(5)]
        Application Layer: (FIR, FIN, Sequence 13, Read)
            Application Control: 0xcd, First, Final(FIR, FIN, Sequence 13)
            Function Code: Read (0x01)
            READ Request Data Objects
                Object(s): Class 0 Data (Obj:60, Var:01) (0x3c01)
```

4. The outstation receives this and calls `validate_rx_link_layer` function to validate the received frame. The following checks are done in this order:
    - Validation of link layer CRC;
    - Validation of frame magic number (0x0564);
    - Conversion from network to host order of dst, src parameters;
    - Validation of destination addresses (must match local address or special addresses from `0xfffc` to `0xffff`)
    - TODO: validation of above layers (received user data) is still not done.

5. If the outstation considers the frame minimally valid (see checks above), it replies with a frame crafted by the `encode_dnp3_read_resp_message` function. The lower link layer is filled out with the following data:
    - magic number, length, and control flags
    - `source = <local address>`;
    - `destination = <source address of the previously received frame>`;

6. Still in the `encode_dnp3_read_resp_message` multiple CRCs are calculated for 16-byte blocks of a dummy application layer. Finally, the link layer + user data is copied to an output buffer which is then sent back to the master through the established TCP socket. The outstation source address is validated at the beginning of program execution and does not need to be validated before transmission.

## Tests and debugs

To run unit tests:

```
make tests
./tests
```

Unit tests cover only a few basic cases to validate some functions.

Clean up with `make clean`. A DEBUG flag may be enabled in the Makefile by adding -DDEBUG in the CFLAGS paremeter:

```diff
diff --git a/Makefile b/Makefile
--- a/Makefile
+++ b/Makefile
@@ -1,5 +1,6 @@
 CC=gcc
-CFLAGS=-I. -Wall
+CFLAGS=-I. -Wall -DDEBUG
 DEPS = dnp3.h dnp3.c

 %.o: %.c $(DEPS)
```

## Known restrictions

At the moment the READ RESPONSE sent from the outstation to the master is not recognized as a DNP3.0 packet by wireshark, there are still improvements to be made.

The application layer was not developed and is mostly configured through static buffers.

The following fields are hard-coded into the programs:
 - The outstation socket binds to any IPv4 address (`INADDR_ANY`). This was only tested with loopback address, though;
 - The master program attempts to connect to loopback IPv4 address only;

# Requirements (in portuguese)
## Objetivo

- Desenvolver um programa que simule uma remota DNP3 que responda a função FC 01 (Read).
- Ao enviar um telegrama o programa deverá executar as seguintes validações: CRC () e o endereço da remota.

Segue exemplo um exemplo de FC (01) do mestre a remota:

### Envio do mestre:

    link layer magic: 0x05 0x64
    link layer len: 0x0b
    link layer ctrl: 0xc4
    link layer dst: 0x46 0x00
    link layer src: 0x40 0x00
    link layer crc: 0xa3 0xfe
    transport layer: 0xd0
    APP data chunk:
       |  app layer ctrl:           0xcd
       |  app layer function code:  0x01 (read)
       |  object type group:        0x3c (Object group 60)
       |  object type variation:    0x02 (variation 2)
       |  qualifier field:          0x06 (always 0x06, see A26.1.2.1)
       |  CRC: 0xc2 0x62

### Resposta da remota:

    link layer:      0x05 0x64 0x33 0x44 0x40 0x00 0x46 0x00 0xe5 0xb4
    transport layer: 0xd0
    app layer:       0xed 0x81 0x06 0x00 0x04 0x02 0x28 0x04 0x00 0xe0 0x00 0x81 (12)
      0x99 0xd7 0x74 0xbd 0xc2 0x10 0x72 0x01 0xdc 0x00 0x81 0x99 0xd7 0x74 0x10 (15)
      0x72 0x01 0xdf 0x00 0x81 0x99 0x1e 0x9a 0xd7 0x74 0x10 0x72 0x01 0xe1 0x00 (15)
      0x81 0x99 0xd7 0x74 0x10 0x72 0x01 0xbd 0xf5                                (9)

## Entrega:
Publique o aplicativo no github, faça pequenos commits para indicar evolução do trabalho ao longo do tempo, descreva no readme como testar o programa e implemente testes unitários.

## Prazo de entrega:
23:59 22/05/2020

## Referências fornecidas:
- https://www.dnp.org/About/Overview-of-DNP3-Protocol
- https://www.electron.com.br/arquivos/artigos-tecnicos/dnp.pdf

## Referências adicionais:
- https://en.wikipedia.org/wiki/DNP3 (overview)
- https://github.com/ITI/ICS-Security-Tools/tree/master/pcaps (capturas de pacotes DNP3 para facilitar o entendimento)
- https://www.ixiacom.com/company/blog/scada-distributed-network-protocol-dnp3 (diagrama do DNP3 sobre IP)
- IEEE 1815-2012 Standard;