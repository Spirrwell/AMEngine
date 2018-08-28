#ifndef IMODEL_HPP
#define IMODEL_HPP

class IModel
{
public:
	virtual ~IModel() = default;

	virtual void Draw() = 0;
	virtual unsigned int GetModelIndex() = 0;
};

#endif // IMODEL_HPP