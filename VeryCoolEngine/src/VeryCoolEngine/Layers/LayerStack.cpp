#include "vcepch.h"
#include "LayerStack.h"

namespace VeryCoolEngine {

	LayerStack::LayerStack(){
	}

	LayerStack::~LayerStack(){
		for (Layer* layer : _layers) delete layer;
	}

	void LayerStack::PushLayer(Layer* layer){
		_layers.emplace(_layers.begin() + _insertIndex, layer);
	}

	void LayerStack::PushOverlay(Layer* overlay){
		_layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer){
		auto it = std::find(_layers.begin(), _layers.end(), layer);
		if (it != _layers.end()){
			_layers.erase(it);
			_insertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay){
		auto it = std::find(_layers.begin(), _layers.end(), overlay);
		if (it != _layers.end()) _layers.erase(it);
	}

}