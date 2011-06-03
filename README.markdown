About
=====

This holds a Statistical Packet Inspection system. Its divided into two parts: libspi - a backend handling the maths -
and spid - frontend deciding what to feed libspi with and what to do with the results.

TODO
====

* libspi
  * review the flow/endpoint closing code - especially in context of learning files
  * make the samples/model the storage, using pcap files to train libspi each time is not nice
    * shared, multiaccess storage could enable parallel spid operation
  * implement SVM
  * implement EWMA verdict issuer
  * let for choosing classifier/verdict issuer and their options
* spid
  * sooner or later a real config file will be necessary :)
  * actions!

libspi events
=============

Below is list of libspi events. They can be also treated as messages.

Subscribe to events with `spi_subscribe()`, announce with `spi_announce()`.

* `endpointPacketsReady(struct spi_ep *ep)` - endpoint accumulated at least C packets (default 80) and
  is ready for classification
* `endpointClassification(struct spi_classresult *cr)` - endpoint packets classified and new result ready
  for decision process
* `endpointVerdictChanged(struct spi_ep *ep)` - verdict about classification changed for this endpoint
* `kisspTraindataUpdated(void)` - learning samples queued, do the model database update
* `classifierModelUpdated(void)` - some samples learned, the model database has changed
* `gcSuggestion(void)` - running garbage collector suggested