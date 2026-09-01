__kernel void upscaler(__read_only image2d_t sourceImage, __write_only image2d_t destinationImage)
{
	int x      = get_global_id(0);
	int y      = get_global_id(1);

	float4 color = read_imagef(sourceImage, (int2) (x, y));

	color.s0 = color.s0 * 10.0;

	write_imagef(destinationImage, (int2) (x, y), color);
}