select_impl - transfer packets within a timeout using select()

Let's transfer an already ready packet from client to server:
> Client calls select() to poll socket for write() within a timeout.
> If timeout has expired, client discarts packet, else (obviously) it sends
  packet to server.
> In server side, server uses select() to wait for client's packet within a
  timeout too.
> If data arrives __in_time__ we use it. ***Any late data we drop***.
----------

###########################################################################
# Currently (09/11/2016) I found it's not so good as I thought (see timer #
# syncing).                                                               #
###########################################################################

Ok, being short:
-> Wait for packet within a specific timeout.
-> If packet arrives too late, drop it based in __sent_timestamp__.
-> Else, do the stuff ...

In IPMic ...
* snd_pcm_avail() should determine the amount of time we wait in select()
* synchronize playback with select()
