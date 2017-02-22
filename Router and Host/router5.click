require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth14, IN_MAC da:78:c5:dc:17:78, OUT_MAC c6:a2:cb:b4:51:0f, LOSS 0.99, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth15, IN_MAC ce:9c:b3:89:3d:63, OUT_MAC 16:8f:d8:4f:ad:a8, LOSS 0.99, DELAY 0.2);

topology :: Topology(MY_ADDRESS 500);
router :: RoutingTable(TOPOLOGY topology, MY_ADDRESS 500);
switch :: PacketSwitch();
forward :: ForwardTable(TOPOLOGY topology, ROUTER router,SWITCH switch, MY_ADDRESS 500);
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
