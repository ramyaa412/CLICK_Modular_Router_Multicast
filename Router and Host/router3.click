require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth5, IN_MAC 3e:eb:8f:ea:81:15, OUT_MAC d6:6c:a1:f0:83:28, LOSS 0.99, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth6, IN_MAC 6a:df:1f:ae:8b:84, OUT_MAC 26:15:2d:7f:21:38, LOSS 0.99, DELAY 0.2);
rp3 :: LossyRouterPort(DEV veth7, IN_MAC be:42:e5:e2:16:db, OUT_MAC 76:c6:a2:cc:bb:f4, LOSS 0.99, DELAY 0.2);

topology :: Topology(MY_ADDRESS 300);
router :: RoutingTable(TOPOLOGY topology, MY_ADDRESS 300);
switch :: PacketSwitch();
forward :: ForwardTable(TOPOLOGY topology, ROUTER router,SWITCH switch,  MY_ADDRESS 300);
classifier1 :: ProjClassifier();
classifier2 :: ProjClassifier();
classifier3 :: ProjClassifier();

rp1 -> classifier1;
classifier1[0] -> [0]topology; // hello
classifier1[1] -> [0]router; // update
classifier1[2] -> [0]forward; // ack
classifier1[3] -> [0]forward; // data
/*
forward[0] -> switch;
router[0] -> switch;
topology[0] -> switch;
*/
rp2 -> classifier2;
classifier2[0] -> [1]topology; // hello
classifier2[1] -> [1]router; // update
classifier2[2] -> [1]forward; // ack
classifier2[3] -> [1]forward; // data

rp3 -> classifier3;
classifier3[0] -> [2]topology; // hello
classifier3[1] -> [2]router; // update
classifier3[2] -> [2]forward; // ack
classifier3[3] -> [2]forward; // data

forward[0] -> switch;
router[0] -> switch;
topology[0] -> switch;

switch[0] -> rp1;
switch[1] -> rp2;
switch[2] -> rp3;