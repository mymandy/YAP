#include "ProcessorImpl.h"

#include <iostream>
#include <algorithm>
#include <assert.h>
#include "Interface/Interfaces.h"
#include "Utilities/CommonMethod.h"
#include "VariableSpace.h"

using namespace std;
using namespace Yap;

namespace Yap
{
namespace _details
{
	class Port : 
		public IPort
	{
		IMPLEMENT_SHARED(Port)
	public:
		Port::Port(const wchar_t * name, unsigned int dimension_count, int data_type) :
			_name(name),
			_dimension_count(dimension_count),
			_data_type(data_type)
		{
		}
	
		virtual const wchar_t * GetId() const override
		{
			return _name.c_str();
		}

		virtual unsigned int GetDimensionCount() const override
		{
			return _dimension_count;
		}

		virtual int GetDataType() const override
		{
			return _data_type;
		}

	protected:
		std::wstring _name;
		unsigned int _dimension_count;
		int _data_type;
	};
};	// namespace Yap::details

	using namespace _details;
	 
	ProcessorImpl::ProcessorImpl(const wchar_t * class_id) :
		_system_variables(nullptr),
		_class_id(class_id),
		_properties(shared_ptr<VariableSpace>(new VariableSpace)),
		_input(YapShared(new PtrContainerImpl<IPort>)),
		_output(YapShared(new PtrContainerImpl<IPort>))
	{
		
	}

	Yap::ProcessorImpl::ProcessorImpl(const ProcessorImpl& rhs) :
		_input(rhs._input->Clone()),
		_output(YapShared<PtrContainerImpl<IPort>>(rhs._output->Clone())),
		_properties(new VariableSpace(*(rhs._properties))),
		_instance_id(rhs._instance_id),
		_class_id(rhs._class_id),
		_system_variables(nullptr),
		_module{rhs._module}
	{
		_links.clear();
		_in_property_mapping.clear();
	}

	ProcessorImpl::~ProcessorImpl()
	{
	}

	IPortContainer * ProcessorImpl::Inputs()
	{
		return _input.get();
	}

	IPortContainer * ProcessorImpl::Outputs()
	{
		return _output.get();
	}

	bool ProcessorImpl::Link(const wchar_t * out_port,
							 IProcessor* next_processor,
							 const wchar_t * in_port)
	{
		assert(wcslen(out_port) != 0);
		assert(next_processor != nullptr);
		assert(wcslen(in_port) != 0);

		if (!CanLink(out_port, next_processor, in_port))
			return false;

		_links.insert(std::make_pair(out_port, Anchor(next_processor, in_port)));
		return true;
	}

	bool ProcessorImpl::AddInput(const wchar_t * name, 
								 unsigned int dimensions,
								 int data_type)
	{
		return _input->Add(name, new Port(name, dimensions, data_type));
	}

	bool ProcessorImpl::AddOutput(const wchar_t * name,
								  unsigned int dimensions, 
								  int data_type)
	{
		return _output->Add(name, new Port(name, dimensions, data_type));
	}

	bool ProcessorImpl::Feed(const wchar_t * out_port, IData * data)
	{
		assert(wcslen(out_port) != 0);
		assert(data != nullptr);

		auto range = _links.equal_range(out_port);
		for (auto iter = range.first; iter != range.second; ++iter)
		{
			auto link = iter->second;
			assert(link.processor != nullptr);
			if (link.processor == nullptr || !link.processor->Input(link.port.c_str(), data))
				return false;
		}

		return true;
	}

	
	IVariableContainer * ProcessorImpl::GetProperties() 
	{
		return _properties->Variables();
	}

	const wchar_t * Yap::ProcessorImpl::GetClassId() const
	{
		return _class_id.c_str();
	}

	void Yap::ProcessorImpl::SetClassId(const wchar_t * id)
	{
		_class_id = id;
	}

	const wchar_t * Yap::ProcessorImpl::GetInstanceId() const
	{
		return _instance_id.c_str();
	}

	void Yap::ProcessorImpl::SetInstanceId(const wchar_t * instance_id)
	{
		_instance_id = instance_id;
	}

	bool Yap::ProcessorImpl::MapProperty(const wchar_t * property_id, const wchar_t * param_id, bool input, bool output)
	{
		if (_properties->Variables()->Find(property_id) == nullptr)
		{
			return false;
		}

		if (input)
		{
			_in_property_mapping.insert(make_pair(wstring(property_id), wstring(param_id)));
		}

		if (output)
		{
			_out_property_mapping.insert(make_pair(wstring(property_id), wstring(param_id)));
		}
		return true;
	}

	bool Yap::ProcessorImpl::SetGlobalVariables(IVariableContainer * source_params)
	{
		if (_system_variables.get() == nullptr || _system_variables->Variables() != source_params)
		{
			try 
			{
				_system_variables = make_shared<VariableSpace>(source_params);
			}
			catch (bad_alloc&)
			{
				return false;
			}
		}

		return true;
	}

	bool Yap::ProcessorImpl::CanLink(const wchar_t * source_output_name,
		IProcessor * next,
		const wchar_t * next_input_name)
	{
		assert(next != nullptr);
		assert(wcslen(source_output_name) != 0);
		assert(wcslen(next_input_name) != 0);

		auto out_port = _output->Find(source_output_name);
		if (out_port == nullptr)
			return false;

		auto in_ports = next->Inputs();
		if (in_ports == nullptr)
			return false;

		auto in_port = in_ports->Find(next_input_name);
		if (in_port == nullptr)
			return false;

		if ((out_port->GetDataType() & in_port->GetDataType()) == 0)
			return false;

		TODO(Refine the processing of YAP_ANY_DIMENSION and support of multiple data type.);

		return (out_port->GetDimensionCount() == in_port->GetDimensionCount() ||
			out_port->GetDimensionCount() == YAP_ANY_DIMENSION ||
			in_port->GetDimensionCount() == YAP_ANY_DIMENSION);
	}

	bool Yap::ProcessorImpl::OutportLinked(const wchar_t * out_port_name) const
	{
		return (_links.find(out_port_name) != _links.end());
	}

	void Yap::ProcessorImpl::SetModule(ISharedObject * module)
	{
		_module = YapShared(module);
	}

	bool Yap::ProcessorImpl::VariablesValid(IData *data, int *slice_index_return,
		int *slice_count_return, int *channel_index_return, int *channel_switch_return) const
	{
		//CString str = TEXT("D:\\temp\\1.jpg");
		//FileSystem::FileExists("D:\\temp\\1.jpg");// ;

		VariableSpace variables(data->GetVariables());

		int slice_index = variables.Get<int>(L"slice_index");
		int slice_count = variables.Get<int>(L"slice_count");
		int channel_index = variables.Get<int>(L"channel_index");
		unsigned int channel_switch = variables.Get<int>(L"channel_switch");
		int channel_count = CommonMethod::GetChannelCountUsed(channel_switch);

		//channel_index < 0 denotes channel merged data.
		//channel max count equals sizeof(unsigned int) * 8;
		//slice count denotes the total number of 1 channel.
		//slice index denote the slice index in a channel.

		int channel_count_max = sizeof(unsigned int) * 8;
		assert(slice_index >= 0);
		assert(slice_index < slice_count);
		assert(slice_count >= 1);
		assert(slice_count <= 200);
		assert(channel_index >= 0);
		assert(channel_index <= channel_count_max);

		if (slice_index_return) {
			*slice_index_return = slice_index;
		}
		if (slice_count_return) {
			*slice_count_return = slice_count;
		}
		if (channel_index_return) {
			*channel_index_return = channel_index;
		}
		if (channel_switch_return) {
			*channel_switch_return = channel_switch;
		}
		return true;
	}

	/**
	\remark Add "Slice_Index" param to IData in pipeline.
	*/
	bool Yap::ProcessorImpl::AddSliceindexParam(IData *data, int index, int data_type)
	{
		
		if (nullptr == data->GetVariables())
		{
			VariableSpace variable;
			//
			if (data_type == Yap::DataTypeComplexFloat)
			{
				dynamic_cast<Yap::DataObject<std::complex<float>>*>(data)->SetVariables(variable.Variables());
			}
			else
			{
				dynamic_cast<Yap::DataObject<unsigned short>*>(data)->SetVariables(variable.Variables());
			}


		}

		VariableSpace variables(data->GetVariables());

		variables.AddVariable(L"int", L"slice_index", L"slice index.");
		variables.Set(L"slice_index", static_cast<int>(index));
		/*
		Yap::IDimensions *dimensions = data->GetDimensions();
		//Yap::Dimensions  *dimensions2 = dynamic_cast<Yap::Dimensions*>(dimensions);
		unsigned int channel_length;
		unsigned int channel_start_index;
		bool result = dynamic_cast<Yap::Dimensions*>(dimensions)->GetDimension2(
								Yap::DimensionChannel,
								channel_length,
								channel_start_index);
			
		if (result && channel_length == 1)
		{
			variables.AddVariable(L"int", L"channel_index", L"channel index.");
			variables.Set(L"channel_index", static_cast<int>(channel_start_index));
		}
		*/
		return true;

	}

	bool Yap::ProcessorImpl::AddASingleVarible(IData *data, wstring variable_name, int value, int data_type)
	{
		if (nullptr == data->GetVariables())
		{
			VariableSpace variable;
			//
			if (data_type == Yap::DataTypeComplexFloat)
			{
				dynamic_cast<Yap::DataObject<std::complex<float>>*>(data)->SetVariables(variable.Variables());
			}
			else if(data_type == Yap::DataTypeShort)
			{
				dynamic_cast<Yap::DataObject<unsigned short>*>(data)->SetVariables(variable.Variables());
			}
			else
			{
				assert(0);
			}


		}

		VariableSpace variables(data->GetVariables());

		variables.AddVariable(L"int", variable_name.c_str(), (variable_name + L":.").c_str());
		variables.Set(variable_name.c_str(), static_cast<int>(value));
		return true;
	}

};	// namepace Yap

