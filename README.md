# SlidingWindowProtocol

Efficient implementation of the Sliding Window Protocol, used to send files between two points (sender and receiver).
'Efficient' in this case means that the "pipeline" of transmission will always be fully used.


The Data link simulator API (provided by our university) could reorder, corrupt or lose packages. All these cases
are taken into consideration and corrected by the protocol.


An experiment could be run using the provided script, setting the following parameters:

 Markup : * SPEED = 5 ... 20 (in Mbps)
          * DELAY = 10 ... 1000 (in ms)
          * LOSS = 0 ... 10 (percentage of lost packages)
          * CORRUPT = 0 ... 10 (percentage of corrupt packages)
          * REORDER = 0 ... 10 (percentage of reordered packages)
          * FILENAME = name of the file to be sent. The received file will be named recv_FILENAME.
