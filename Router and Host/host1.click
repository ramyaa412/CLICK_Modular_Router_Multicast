require(library /home/comnetsii/elements/lossyrouterport.click);

rp::LossyRouterPort(DEV veth1, IN_MAC 0a:53:2c:68:11:23 , OUT_MAC be:94:32:34:48:a7 , LOSS 0.99, DELAY 0.2);
client::Client( PAYLOAD "COMPLEX", K 3, MY_ADDRESS 1, DST1 5, DST2 6, DST3 7, DELAY 10);
classifier :: ProjClassifier();
client -> Print(Sending, MAXLENGTH -1, CONTENTS ASCII) -> rp;
rp -> classifier;
classifier[0] -> Discard(); //hello packets discarded
classifier[1] -> Discard(); //update packets discarded
classifier[2] -> [1]client; //ack
classifier[3] -> [0]client; //data