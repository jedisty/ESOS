#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bitmap.h>

unsigned long bitmap;
#define BYTE_PER_BIT	8
int __init bitmap_init(void){

	printk("kernel module for bitmap test by sslee\n");
		
	int bits = sizeof(bitmap) * BYTE_PER_BIT;
	int idx = 0; 

	bitmap_zero(&bitmap, bits);

	idx = find_first_zero_bit(&bitmap, bits);
	printk("first zero bit = %d\n", idx);
/*
	bitmap_fill(&bitmap, bits);

	idx = find_first_zero_bit(&bitmap, bits);
	printk("first zero bit = %d\n", idx);
*/
	bitmap_set(&bitmap, 3, 1);

	idx = find_first_bit(&bitmap, bits);
	printk("first zero bit = %d\n", idx);


//	bitmap_zero(&bitmap, bits);
//	printk("is empty = %d\n", bitmap_empty(&bitmap, bits));
	printk("is empty = %d\n", test_bit(2, &bitmap));
	printk("is empty = %d\n", test_bit(3, &bitmap));
	printk("is empty = %d\n", test_bit(4, &bitmap));
	

	return 0;

}
void __exit bitmap_exit(void){

	printk("terminating bitmap test\n");
}
module_init(bitmap_init);
module_exit(bitmap_exit);
MODULE_LICENSE("GPL");
