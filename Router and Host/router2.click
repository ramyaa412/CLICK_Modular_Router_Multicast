require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth10, IN_MAC 26:15:2d:7f:21:38, OUT_MAC 6a:df:1f:ae:8b:84, LOSS 0.99, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth11, IN_MAC 2a:60:79:13:a8:72, OUT_MAC be:bb:04:45:f0:5d, LOSS 0.99, DELAY 0.2);

topology :: Topology(MY_ADDRESS 200);
router :: RoutingTable(TOPOLOGY topology, MY_ADDRESS 200);
switch :: PacketSwitch();
forward :: ForwardTable(TOPOLOGY topology, ROUTER router,SWITCH switch, MY_ADDRESS 200);

classifier1 :: ProjClassifier();
classifier2 :: ProjClassifier();

rp1 -> classifier1;
classifier1[0] -> [0]topology; // hello
classifier1[1] -> [0]router; // update
classifier1[2] -> [0]forward; // ack
classifier1[3] -> [0]forward; // data

rp2 -> classifier2;
classifier2[0] -> [1]topology; // hello
classifier2[1] -> [1]router; // update
classifier2[2] -> [1]forward; // ack
classifier2[3] -> [1]forward; // data

forward[0] -> switch;
router[0] -> switch;
topology[0] -> switch;

switch[0] -> rp1;
switch[1] -> rp2;
