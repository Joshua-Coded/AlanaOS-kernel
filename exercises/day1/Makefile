# tell the kernel what to build

obj-m += first_module.o

# default target

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
# clean target

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
