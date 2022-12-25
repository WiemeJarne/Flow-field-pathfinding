#ifndef BASE_APPLICATION_H
#define BASE_APPLICATION_H
//-----------------------------------------------------------------
// Application Base
//-----------------------------------------------------------------
class IApp
{
public:
	//Constructor & Destructor
	IApp() = default;
	virtual ~IApp() = default;

	//App Functions
	virtual void Start() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float deltaTime) const = 0;

protected:
	void RenderWorldBounds(float bounds) const
	{
		std::vector<Elite::Vector2> points =
		{
			{ 0, bounds },
			{ bounds, bounds },
			{ bounds, 0 },
			{ 0, 0 }
		};
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
	}

private:
	//C++ make the class non-copyable
	IApp(const IApp&){};
	IApp& operator=(const IApp&){};
};
#endif