#include <lighting.ogsl>

ConstArray(0, nodes, NodeSystem, Node);
ConstArray(1, nodeIds, NodeIds, u32);

Uniform(2, global, Global, { Vec3u padding; ViewHandle view; });
Uniform(3, views, Views_noalloc, { ViewBuffer data; });

In(0, inPosition, Vec3);
In(1, inUv, Vec2);
In(2, inNormal, Vec3);

Out(0, uv, Vec2);
Out(1, normal, Vec3);
Out(2, material, MaterialHandle);

Vertex() {

	u32 nodeId = nodeIds[instanceId];
	Node node = nodes[nodeId];

	Vec3 wpos = transformVert(node, inPosition);
	
	View view = views.data.views[global.view];

    vPosition = mul(view.vp, Vec4(wpos, 1));
	uv = inUv;
	normal = transformDir(node, inNormal);
	material = node.localId + 1;

}