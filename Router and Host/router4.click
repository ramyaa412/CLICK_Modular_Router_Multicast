require(library /home/comnetsii/elements/lossyrouterport.click);

rp1 :: LossyRouterPort(DEV veth8, IN_MAC de:29:48:85:2e:a9, OUT_MAC 4e:6c:99:1f:f5:2e, LOSS 0.99, DELAY 0.2);
rp2 :: LossyRouterPort(DEV veth9, IN_MAC c6:a2:cb:b4:51:0f, OUT_MAC da:78:c5:dc:17:78, LOSS 0.99, DELAY 0.2);

topology :: Topology(MY_ADDRESS 400);
router :: RoutingTable(TOPOLOGY topology, MY_ADDRESS 400);
switch :: PacketSwitch();
forward :: ForwardTable(TOPOLOGY topology, ROUTER router,SWITCH switch, MY_ADDRESS 400);
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
