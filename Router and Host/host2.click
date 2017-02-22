require(library /home/comnetsii/elements/lossyrouterport.click);

rp :: LossyRouterPort(DEV veth12, IN_MAC be:bb:04:45:f0:5d, OUT_MAC 2a:60:79:13:a8:72, LOSS 0.9, DELAY 0.2);

client::Client( PAYLOAD "COMPLEX", K 3, 
                MY_ADDRESS 5, 
                DST1 5, DST2 6, DST3 7,
                DELAY 10);

classifier :: ProjClassifier();

client -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp;

rp -> classifier;

classifier[0] -> Discard(); // client doesn't need hello packets
classifier[1] -> Discard(); // client doesn't need update packets

classifier[2] -> [1]client; // ack packets to client's input 1
classifier[3] -> [0]client; // data packet to client's input 0