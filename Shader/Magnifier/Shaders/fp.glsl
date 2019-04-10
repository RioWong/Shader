 uniform sampler2D g_textureImg1; //����1
 uniform sampler2D g_textureImg2; //����2
 
 uniform float g_centerPosX;//�Ŵ�����Ļ�е�����
 uniform float g_centerPosY;
 uniform float g_radis;//Բ�εİ뾶
 
 varying vec2 g_pPosition; //��ǰ������Ļ�е�����
 
 void main()
 {
	vec2 centerPosition = vec2(g_centerPosX, g_centerPosY); //�Ŵ���ͶӰ����ϵ�µ�λ��
	float disToCenter = distance(g_pPosition, centerPosition);//��ǰ�������ĵ�ľ���

	if(disToCenter>g_radis ) //����Բ�ηŴ󾵣����뷶Χ�ڰ뾶���ڶ����滭��������������&& disToCenter < g_radis * 2.0
		discard;
	
	vec4 color1 = texture2D(g_textureImg1, gl_TexCoord[0].st);
	vec4 color2 = texture2D(g_textureImg1, gl_TexCoord[0].st);
	vec4 color = vec4(255,0,0,1);//color1.rgba*color2.r ; //��ȡ������ɫ

	gl_FragColor =color; //��ɫ
 }
