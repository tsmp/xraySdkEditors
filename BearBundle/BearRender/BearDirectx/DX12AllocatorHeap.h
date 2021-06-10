#pragma once
struct  DX12AllocatorHeapItem
{
	DX12AllocatorHeapItem() :Id(0), Size(0) {}
	ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
	bsize Id;
	bsize Size;
};

template<D3D12_DESCRIPTOR_HEAP_TYPE heap_type, bsize elements_in_heap=512,bool ShaderVisible = true>
class  DX12AllocatorHeap
{
public:
	DX12AllocatorHeap() {}
	inline ~DX12AllocatorHeap()
	{

	}
	inline void clear()
	{

		for (bsize i = 0; i < HeapsOfAddres.size(); i++)
		{
			HeapsOfAddres[i]->DescriptorHeap.Reset();
#ifdef DEBUG
			BEAR_CHECK(HeapsOfAddres[i]->MaxSize == elements_in_heap);
#endif
			bear_delete(HeapsOfAddres[i]);
		}

	}
	inline DX12AllocatorHeapItem allocate(bsize count_element, ID3D12DeviceX* Device)
	{
		BEAR_CHECK(elements_in_heap >=count_element);
		DX12AllocatorHeapItem Result;
		Heap* HeapPtr = 0;
		if(HeapsOfMaxSize.size())
		{
			auto Item = std::lower_bound(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), count_element, [](Heap* a1, bsize a2)->bool {return a1->MaxSize < a2; });
			if (Item == HeapsOfMaxSize.end())
			{
				HeapPtr = create_block(Device);
			}
			else
			{

				HeapPtr = *Item;
			}
		
		}
		else
		{
			HeapPtr = create_block(Device);
		}
		if (HeapPtr->MaxSize < count_element)
		{
			HeapPtr = create_block(Device);
		}
		Result.DescriptorHeap = HeapPtr->DescriptorHeap;
		bsize id = find_id(count_element, HeapPtr);
		Result.Id = id;
		Result.Size = count_element;
		bear_fill(&HeapPtr->BlockInfo[id], count_element,1);
		(HeapPtr)->MaxSize = get_max_size(HeapPtr);
		bear_sort(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), [](Heap* a1, Heap* a2)->bool {return a1->MaxSize < a2->MaxSize; });

		return Result;
	}
	inline void free(DX12AllocatorHeapItem& item)
	{
		if (item.Size == 0)return;
		auto Item = std::lower_bound(HeapsOfAddres.begin(), HeapsOfAddres.end(), item, [](const Heap* a1,const DX12AllocatorHeapItem& a2)->bool {return a1->DescriptorHeap.Get() < a2.DescriptorHeap.Get(); });
		BEAR_CHECK(Item != HeapsOfAddres.end());
		BEAR_CHECK((*Item)->DescriptorHeap.Get() == item.DescriptorHeap.Get());
		bear_fill(&(*Item)->BlockInfo[item.Id], item.Size,0);
		(*Item)->MaxSize = get_max_size(*Item);
		bear_sort(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), [](Heap* a1, Heap* a2)->bool {return a1->MaxSize < a2->MaxSize; });
		item.DescriptorHeap.Reset();
		item.Id = 0;
		item.Size = 0;
	}
private:
	struct Heap
	{
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		uint8 BlockInfo[elements_in_heap];
		bsize MaxSize;
	};
	inline Heap* create_block(ID3D12DeviceX*Device)
	{
		Heap* HeapPtr = bear_new<Heap>();

		{
			D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
			HeapDesc.NumDescriptors = static_cast<UINT>(elements_in_heap);
			HeapDesc.Flags = ShaderVisible?D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			HeapDesc.Type = heap_type;
			R_CHK(Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&HeapPtr->DescriptorHeap)));
		}
		bear_fill(HeapPtr->BlockInfo, elements_in_heap,0);
		HeapPtr->MaxSize = elements_in_heap;
		auto item1 = std::lower_bound(HeapsOfAddres.begin(), HeapsOfAddres.end(), HeapPtr, [](Heap*a1, Heap* a2)->bool {return a1->DescriptorHeap.Get() < a2->DescriptorHeap.Get(); });
		HeapsOfAddres.insert(item1, HeapPtr);
		auto item2 = std::lower_bound(HeapsOfMaxSize.begin(), HeapsOfMaxSize.end(), elements_in_heap, [](Heap* a1, bsize a2)->bool {return a1->MaxSize < a2; });
		HeapsOfMaxSize.insert(item2, HeapPtr);
		return HeapPtr;

	}
	inline bsize get_size(Heap* heap, bsize id)
	{
		bsize Count = 0;
		for (bsize i = id; i < elements_in_heap; i++)
		{
			if (heap->BlockInfo[i]==0)
			{
				Count++;
			}
			else
			{
				return Count;
			}
		}
		return Count;
	}
	inline bsize find_id(bsize size,Heap* heap)
	{
		for (bsize i = 0; i < elements_in_heap;)
		{
			bsize FSize = get_size(heap, i);
			if (FSize)
			{
				if (FSize >= size)
				{
					return i;
				}
				i += FSize;
			}
			else
			{
				i++;
			}
		}
		BEAR_CHECK(0);
		return 0;
	}
	inline bsize get_max_size(Heap* heap)
	{
		bsize MaxSize = 0;
		for (bsize i = 0; i < elements_in_heap;)
		{
			bsize FSize = get_size(heap, i);
			if (FSize)
			{
				MaxSize = BearMath::max(FSize, MaxSize);
				i += FSize;
			}
			else
			{
				i++;
			}
	
		}

		return MaxSize;
	}
	BearVector<Heap*> HeapsOfAddres;
	BearVector<Heap*> HeapsOfMaxSize;
};