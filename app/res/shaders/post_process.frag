In(0, uv, Vec2);
Out(0, outColor, Vec4);

Sampler(0, linear);
Texture2D(1, tex);
Uniform(2, settings, PostProcessingSettings, { Vec2 padding; f32 exposure; f32 gamma; });

Fragment() {
	
    //Sample color
	Vec3 col = sample2D(linear, tex, uv).rgb;
	
    //Exposure & gamma correction
    col = pow(Vec3(1) - exp(-col * settings.exposure), Vec3(1.0f / settings.gamma));
	
    //Write to back buffer
    outColor = Vec4(col, 1);

}