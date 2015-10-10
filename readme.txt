Implementation of secure online voting protocol.



The IP addresses of registrar, Public key distributor, authenticator and counter are stored in the ip.config file

1. Registration Phase:

The registrar runs on as the server and registers each voter by registering its identification details with the its private/public key pair.

The voter generates a private/public key pair and registers it with the registrar.

Run the registrar.c at a server and voters can connect to it as clients by running voter_reg.c.

2. Voting Phase:

The voter casts his vote, encrypts it with public key of the counter and sends it to the authenticator who inturn verifies if it is a legitimate vote and sends a receipt.

Run authenticator_voting.c at a server and voters can connect to it as clients using voter_vote.c

3. Counting Phase

The authenticator removes the identification details from the ballot and sends it to the counter. The counter sends a receipt on receiving the vote. The counter then decrypts the ballots and counts the votes.

Run counter.c at a server and authenticator_count.c connects to it.


## The various parties can request public keys from public key distributor as and when required.