 uniform sampler2D g_textureImg1; //纹理1
 uniform sampler2D g_textureImg2; //纹理2
 
 uniform float g_centerPosX;//放大镜在屏幕中的坐标
 uniform float g_centerPosY;
 uniform float g_radis;//圆形的半径
 
 varying vec2 g_pPosition; //当前点在屏幕中的坐标
 
 void main()
 {
	vec2 centerPosition = vec2(g_centerPosX, g_centerPosY); //放大镜在投影坐标系下的位置
	float disToCenter = distance(g_pPosition, centerPosition);//当前点离中心点的距离

	if(disToCenter>g_radis ) //绘制圆形放大镜，距离范围在半径以内都将绘画出来，否则抛弃&& disToCenter < g_radis * 2.0
		discard;
	
	vec4 color1 = texture2D(g_textureImg1, gl_TexCoord[0].st);
	vec4 color2 = texture2D(g_textureImg1, gl_TexCoord[0].st);
	vec4 color = vec4(255,0,0,1);//color1.rgba*color2.r ; //获取纹理颜色

	gl_FragColor =color; //着色
 }
