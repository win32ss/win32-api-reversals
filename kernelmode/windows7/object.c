OBJECT_TYPE* NTAPI ObGetObjectType(PVOID Object)
{
  return OBJECT_TO_OBJECT_TYPE(Object)->Type;
}