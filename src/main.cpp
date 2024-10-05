#include <iostream>
#include <BASIS/app.h>
#include <imgui.h>
#include <cassert>

using namespace BASIS;
/*
class Sphere
{
	std::vector<>
};
public class SphereMesh {

	public readonly Vector3[] Vertices;
	public readonly int[] Triangles;
	public readonly int Resolution;

	// Internal:
	FixedSizeList<Vector3> vertices;
	FixedSizeList<int> triangles;
	int numDivisions;
	int numVertsPerFace;

	// Indices of the vertex pairs that make up each of the initial 12 edges
	static readonly int[] vertexPairs = { 0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3, 3, 4, 4, 1, 5, 1, 5, 2, 5, 3, 5, 4 };
	// Indices of the edge triplets that make up the initial 8 faces
	static readonly int[] edgeTriplets = { 0, 1, 4, 1, 2, 5, 2, 3, 6, 3, 0, 7, 8, 9, 4, 9, 10, 5, 10, 11, 6, 11, 8, 7 };
	// The six initial vertices
	static readonly Vector3[] baseVertices = { up, left, back, right, forward, down };

	public SphereMesh (int resolution) {
		this.Resolution = resolution;
		numDivisions = Mathf.Max (0, resolution);
		numVertsPerFace = ((numDivisions + 3) * (numDivisions + 3) - (numDivisions + 3)) / 2;
		int numVerts = numVertsPerFace * 8 - (numDivisions + 2) * 12 + 6;
		int numTrisPerFace = (numDivisions + 1) * (numDivisions + 1);

		vertices = new FixedSizeList<Vector3> (numVerts);
		triangles = new FixedSizeList<int> (numTrisPerFace * 8 * 3);

		vertices.AddRange (baseVertices);

		// Create 12 edges, with n vertices added along them (n = numDivisions)
		Edge[] edges = new Edge[12];
		for (int i = 0; i < vertexPairs.Length; i += 2) {
			Vector3 startVertex = vertices.items[vertexPairs[i]];
			Vector3 endVertex = vertices.items[vertexPairs[i + 1]];

			int[] edgeVertexIndices = new int[numDivisions + 2];
			edgeVertexIndices[0] = vertexPairs[i];

			// Add vertices along edge
			for (int divisionIndex = 0; divisionIndex < numDivisions; divisionIndex++) {
				float t = (divisionIndex + 1f) / (numDivisions + 1f);
				edgeVertexIndices[divisionIndex + 1] = vertices.nextIndex;
				vertices.Add (Slerp (startVertex, endVertex, t));
			}
			edgeVertexIndices[numDivisions + 1] = vertexPairs[i + 1];
			int edgeIndex = i / 2;
			edges[edgeIndex] = new Edge (edgeVertexIndices);
		}

		// Create faces
		for (int i = 0; i < edgeTriplets.Length; i += 3) {
			int faceIndex = i / 3;
			bool reverse = faceIndex >= 4;
			CreateFace (edges[edgeTriplets[i]], edges[edgeTriplets[i + 1]], edges[edgeTriplets[i + 2]], reverse);
		}

		Vertices = vertices.items;
		Triangles = triangles.items;
	}

	void CreateFace (Edge sideA, Edge sideB, Edge bottom, bool reverse) {
		int numPointsInEdge = sideA.vertexIndices.Length;
		var vertexMap = new FixedSizeList<int> (numVertsPerFace);
		vertexMap.Add (sideA.vertexIndices[0]); // top of triangle

		for (int i = 1; i < numPointsInEdge - 1; i++) {
			// Side A vertex
			vertexMap.Add (sideA.vertexIndices[i]);

			// Add vertices between sideA and sideB
			Vector3 sideAVertex = vertices.items[sideA.vertexIndices[i]];
			Vector3 sideBVertex = vertices.items[sideB.vertexIndices[i]];
			int numInnerPoints = i - 1;
			for (int j = 0; j < numInnerPoints; j++) {
				float t = (j + 1f) / (numInnerPoints + 1f);
				vertexMap.Add (vertices.nextIndex);
				vertices.Add (Slerp (sideAVertex, sideBVertex, t));
			}

			// Side B vertex
			vertexMap.Add (sideB.vertexIndices[i]);
		}

		// Add bottom edge vertices
		for (int i = 0; i < numPointsInEdge; i++) {
			vertexMap.Add (bottom.vertexIndices[i]);
		}

		// Triangulate
		int numRows = numDivisions + 1;
		for (int row = 0; row < numRows; row++) {
			// vertices down left edge follow quadratic sequence: 0, 1, 3, 6, 10, 15...
			// the nth term can be calculated with: (n^2 - n)/2
			int topVertex = ((row + 1) * (row + 1) - row - 1) / 2;
			int bottomVertex = ((row + 2) * (row + 2) - row - 2) / 2;

			int numTrianglesInRow = 1 + 2 * row;
			for (int column = 0; column < numTrianglesInRow; column++) {
				int v0, v1, v2;

				if (column % 2 == 0) {
					v0 = topVertex;
					v1 = bottomVertex + 1;
					v2 = bottomVertex;
					topVertex++;
					bottomVertex++;
				} else {
					v0 = topVertex;
					v1 = bottomVertex;
					v2 = topVertex - 1;
				}

				triangles.Add (vertexMap.items[v0]);
				triangles.Add (vertexMap.items[(reverse) ? v2 : v1]);
				triangles.Add (vertexMap.items[(reverse) ? v1 : v2]);
			}
		}

	}

	// Convenience classes:

	public class Edge {
		public int[] vertexIndices;

		public Edge (int[] vertexIndices) {
			this.vertexIndices = vertexIndices;
		}
	}

	public class FixedSizeList<T> {
		public T[] items;
		public int nextIndex;

		public FixedSizeList (int size) {
			items = new T[size];
		}

		public void Add (T item) {
			items[nextIndex] = item;
			nextIndex++;
		}

		public void AddRange (IEnumerable<T> items) {
			foreach (var item in items) {
				Add (item);
			}
		}
	}

}*/
auto populatePipeline(std::uint64_t pipeHash)
{
VertexInputState vstate = 
{
	VertexBinding
	{
		.location = 0,
		.binding  = 0,
		.offset   = offsetof(Vertex,pos),
		.fmt	  = Format::RGB32F
	},
	VertexBinding
	{
		.location = 1,
		.binding  = 0,
		.offset   = offsetof(Vertex,uv),
		.fmt 	  = Format::RG32F
	},
	VertexBinding
	{
		.location = 2,
		.binding  = 0,
		.offset   = offsetof(Vertex,color),
		.fmt 	  = Format::RGBA32F
	},
};

	auto vert = Shader(ShaderType::VERTEX  , App::loadFile("..\\shaders\\default.vert"),"dv");
	auto frag = Shader(ShaderType::FRAGMENT, App::loadFile("..\\shaders\\default.frag"),"df");
	
	auto defInfo = PipelineCreateInfo
	{
		.vertex = &vert,
		.fragment = &frag,
	};
	defInfo.vertexState = std::vector(vstate.begin(),vstate.begin()+3);
	switch(pipeHash)
	{
		case "default"_hash: return Pipeline(defInfo,"defaultPipeline");
		default: assert("no pipeline");
	};
}
struct globalUniforms
{
	glm::mat4 model{1.f};
	glm::mat4 viewProj{1.f};
} g_uniforms;

struct SampleApp final : public App
{
	Renderer rndr;
	Manager manager;
	Pipeline defPipe;
	TypedBuffer<globalUniforms> uBuf;
	
	SampleApp(const AppCreateInfo& inf) : 
	App(inf),
	defPipe(populatePipeline("default"_hash)),
	uBuf(g_uniforms,BufferFlags::DYNAMIC)
	{
		Renderer::enableCapability(Cap::DEPTH_TEST);
		Renderer::enableCapability(Cap::FRAMEBUFFER_SRGB);
		/*
		std::uint32_t color_renderbuffer;
		glGenRenderbuffers(1, &color_renderbuffer);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)color_renderbuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, fbo_width, fbo_height );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// Build the texture that will serve as the depth attachment for the framebuffer.
		GLuint depth_renderbuffer;
		glGenRenderbuffers(1, &depth_renderbuffer);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)depth_renderbuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fbo_width, fbo_height );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// Build the framebuffer.
		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			// Error

		glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
		
		manager.materialUploadCallback = 
		[](const std::vector<Material>& materials)
		{

			struct alignas(16) ShaderMaterial
			{
				glm::vec4 baseColorFactor;
				std::uint64_t baseColorTexture;
				float alphaMask;
				float alphaCutoff;
				std::uint32_t flags;
			};
			std::vector<ShaderMaterial> dstMaterials;
			dstMaterials.reserve(materials.size());
			for(const auto& m : materials)
			{
				dstMaterials.emplace_back(
				m.baseColorFactor,
				m.baseColorTexture,
				m.alphaMask,
				m.alphaCutoff,
				m.flags);
			}
			return Buffer(std::span<ShaderMaterial>(dstMaterials),0);
		};
		auto m1 = manager.getModel("mage"_hash,"../assets/mage.glb",false);
		
		
		rndr.clearColor(0.1,0.1,0.1,1);
	}
	void render(double delta) override
	{
		rndr.BeginFrame();
		
		rndr.clear(MaskFlagBit::COLOR | MaskFlagBit::DEPTH);
		rndr.bindUniformBuffer(uBuf,1);
		rndr.clear(MaskFlagBit::COLOR | MaskFlagBit::DEPTH);
		
		// maybe make fov and zFar adjustible?
		auto projection = glm::perspective( glm::radians( 90.f ) , float( m_width / m_height ) , 0.1f , 100.0f );
		g_uniforms.viewProj = projection * m_camera.view();
		
		rndr.bindPipeline(defPipe);
		rndr.drawModel(manager.getModel("mage"_hash));
		uBuf.update(g_uniforms);
		rndr.EndFrame();
	}
};
auto main() -> decltype(1+1)
{	
	SampleApp test({
		.name = "test",
		.width = 800,
		.height = 800,
		.flags = AppFlags::DEBUG | AppFlags::DOUBLEBUFFER | AppFlags::SRGB | AppFlags::NO_VSYNC});		
	test.run();
	
}
