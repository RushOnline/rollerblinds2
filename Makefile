DEVICE	= rollerlinds
CONFIG	= rb.yaml

all: compile

%:
	esphome $(CONFIG) $@

.PHONY: all
