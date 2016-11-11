What is pthread_impl (POSIX Threads Implementation)?

This implementation is related with timer_syncing, because timer_syncing
sends to us the signals.

We simply wait for signals (pthread mutex cond?) and do our stuff.
