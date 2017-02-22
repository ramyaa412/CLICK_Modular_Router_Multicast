require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth2, IN_MAC be:94:32:34:48:a7, OUT_MAC 0a:53:2c:68:11:23, LOSS 0.99, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth3, IN_MAC d6:6c:a1:f0:83:28, OUT_MAC 3e:eb:8f:ea:81:15, LOSS 0.99, DELAY 0.2);
rp3 :: LossyRouterPort(DEV veth4, IN_MAC 4e:6c:99:1f:f5:2e, OUT_MAC de:29:48:85:2e:a9, LOSS 0.99, DELAY 0.2);

topology :: Topology(MY_ADDRESS 100);
router :: RoutingTable(TOPOLOGY topology, MY_ADDRESS 100);
switch :: PacketSwitch();
forward :: ForwardTable(TOPOLOGY topology, ROUTER router, SWITCH switch, MY_ADDRESS 100);

classifier1 :: ProjClassifier();
classifier2 :: ProjClassifier();
classifier3 :: ProjClassifier();

rp1 -> classifier1;
classifier1[0] -> [0]topology; // hello
classifier1[1] -> [0]router; // update
classifier1[2] -> [0]forward; // ack
classifier1[3] -> [0]forward; // data

rp2 -> classifier2;
classifier2[0]-> [1]topology; // hello
classifier2[1]-> [1]router; // update
classifier2[2]-> [1]forward; //ack
classifier2[3]-> [1]forward;// data

rp3 -> classifier3;
classifier3[0]-> [2]topology; // hello
classifier3[1]-> [2]router; // update
classifier3[2]-> [2]forward; //ack
classifier3[3]-> [2]forward;// data

forward[0] -> switch;
router[0] -> switch;
topology[0] -> switch;

switch[0] -> rp1;
switch[1] -> rp2;
switch[2] -> rp3;