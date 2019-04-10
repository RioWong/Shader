varying vec2 g_pPosition; //当前点在屏幕中的坐标

void main()
{
	vec4 pjPosition = vec4(gl_ModelViewProjectionMatrix * gl_Vertex); //将当前点转换到屏幕坐标系下
	g_pPosition = vec2(pjPosition.x/pjPosition.w, pjPosition.y/pjPosition.w); 

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}
