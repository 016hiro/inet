
#include "MessageChecker.h"

Define_Module(MessageChecker);


MessageChecker::MessageChecker()
: forwardedMsg(0)
, checkedMsg(0)
{
}

void MessageChecker::initialize()
{
	cXMLElement* root = par("config");
	if (std::string(root->getTagName()) == "checker")
	{
		m_checkingInfo = root->getChildren();
		m_iterChk = m_checkingInfo.begin();
	}
	forwardedMsg = 0;
	checkedMsg = 0;
	WATCH(forwardedMsg);
	WATCH(checkedMsg);
}

void MessageChecker::handleMessage(cMessage* msg)
{
	forwardedMsg++;
	checkMessage(msg);
	forwardMessage(msg);
}

void MessageChecker::checkMessage(cMessage* msg)
{
	while (m_iterChk != m_checkingInfo.end())
	{
		cXMLElement& messagePattern = **m_iterChk;

		if (std::string(messagePattern.getTagName()) == "message" && messagePattern.hasAttributes())
		{
			int occurence = atol(messagePattern.getAttribute("occurence"));
			if (occurence > 0)
			{
				if (messagePattern.hasChildren())
					checkFields(msg, msg->getDescriptor(), messagePattern.getChildren());

				occurence--;
				std::ostringstream occur_str;
				occur_str << occurence;
				messagePattern.setAttribute("occurence", occur_str.str().data());
				checkedMsg++;
			}
			if (occurence == 0)
				m_iterChk++;
			break;
		}
		else
			m_iterChk++;		
	}
}

void MessageChecker::checkFields(void* object, cClassDescriptor* descriptor, const cXMLElementList& msgPattern) const
{
	// fldPatternList contains the list of fields to be inspected
	cXMLElementList::const_iterator iter = msgPattern.begin();
	while (iter != msgPattern.end())
	{
		const cXMLElement& pattern = **iter;
		cXMLAttributeMap attr = pattern.getAttributes();
		std::string patternType(pattern.getTagName());

		// find field position into the client object
		int field = findFieldIndex(object, descriptor, attr["name"]);

		// check the field type into the client object (if requiered)
		if (attr.find("type") != attr.end())
			checkFieldType(object, descriptor, field, attr);

		if (patternType == "fieldValue")
			checkFieldValue(object, descriptor, field, attr);
		else if (patternType == "fieldObject")
			checkFieldObject(object, descriptor, field, attr, pattern);
		else if (patternType == "fieldArray")
			checkFieldArray(object, descriptor, field, attr);
		else if (patternType == "fieldValueInArray")
			checkFieldValueInArray(object, descriptor, field, attr);
		else if (patternType == "fieldObjectInArray")
			checkFieldObjectInArray(object, descriptor, field, attr, pattern);

		iter++;
	}
}

void MessageChecker::checkFieldValue(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, int i) const
{
	char buf[BUFSIZE];
	// get the field string value from the client object
	descriptor->getFieldAsString(object, field, i, buf, BUFSIZE);
	std::string value(buf);

	// convert the field value into its name of enum value
	if (descriptor->getFieldProperty(object, field, "enum"))
	{
		cEnum* enm = cEnum::find(descriptor->getFieldProperty(object, field, "enum"));
		if (enm) value = enm->getStringFor(atol(buf));
	}

	// check field value into the client object
	if (value.find(attr["value"])!=0) //allow to keep reference values even if
									 //simtime precision changed...
		opp_error("mismatch field \"%s\" into the message %d (\"%s\" != \"%s\")", 
			attr["name"].data(), forwardedMsg, value.data(), attr["value"].data());
}

void MessageChecker::checkFieldObject(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, const cXMLElement& pattern, int i) const
{
	// get the client object associated to the field
	void* obj = descriptor->getFieldStructPointer(object, field, i);

	// get the client object associated to the field, and its descriptor class
	cClassDescriptor* descr = descriptor->getFieldIsCObject(object, field) ? 
		cClassDescriptor::getDescriptorFor((cPolymorphic*)obj) :
		cClassDescriptor::getDescriptorFor(descriptor->getFieldStructName(object, field));

	checkFields(obj, descr, pattern.getChildren());
}

int MessageChecker::checkFieldArray(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr) const
{
	if (!descriptor->getFieldIsArray(object, field))
		opp_error("the field \"%s\" into the message %d isn't an array", attr["name"].data(), forwardedMsg);

	// check the size of the field array into the client object
	int size = atol(attr["size"].data());
	int fieldSize = descriptor->getArraySize(object, field);
	if (size != fieldSize)
		opp_error("field array \"%s\" contains %d element(s) (and not %d) into message %d",
			attr["name"].data(), fieldSize, size, forwardedMsg);

	return fieldSize;
}

void MessageChecker::checkFieldValueInArray(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr) const
{
	int fieldSize = checkFieldArray(object, descriptor, field, attr);

	// convert attribute "index" into integer
	int i = atol(attr["index"].data());

	if (i >= fieldSize)
		opp_error("the field \"%s\" into the message %d has no entry for index %d", attr["name"].data(), forwardedMsg, i);

	// check field value into the client object
	checkFieldValue(object, descriptor, field, attr, i);
}

void MessageChecker::checkFieldObjectInArray(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, const cXMLElement& pattern) const
{
	int fieldSize = checkFieldArray(object, descriptor, field, attr);

	// convert attribute "index" into integer
	int i = atol(attr["index"].data());

	if (i >= fieldSize)
		opp_error("the field \"%s\" into the message %d has no entry for index %d", attr["name"].data(), forwardedMsg, i);

	// check field object into the client object
	checkFieldObject(object, descriptor, field, attr, pattern, i);
}

void MessageChecker::checkFieldType(void* object, cClassDescriptor* descriptor, int field, cXMLAttributeMap& attr, int i) const
{
	std::string type;

	if (descriptor->getFieldIsCObject(object, field))
		type = ((cPolymorphic*)descriptor->getFieldStructPointer(object, field, i))->getClassName();
	else
		type = descriptor->getFieldTypeString(object, field);

	if (type != attr["type"])
		opp_error("mismatch type for the field \"%s\" into message %d (\"%s\" != \"%s\")",
			attr["name"].data(), forwardedMsg, type.data(), attr["type"].data());
}

int MessageChecker::findFieldIndex(void* object, cClassDescriptor* descriptor, const std::string& fieldName) const
{
	std::ostringstream availableFields;
	for (int i = 0; i < descriptor->getFieldCount(object); i++)
	{
		availableFields << descriptor->getFieldName(object, i) << ", ";
		if (std::string(descriptor->getFieldName(object, i)) == fieldName)
			return i;
	}

	opp_error("unknown field \"%s\" into message %d\nAvailable fields in \"%s\" are : %s"
		, fieldName.data(), forwardedMsg, descriptor->getClassName(), availableFields.str().data());
	return 0;
}

void MessageChecker::forwardMessage(cMessage* msg)
{
	cGate* gateOut = gate("out");
	cChannel* channel = gateOut->getChannel();
	simtime_t now = simTime();
	simtime_t endTransmissionTime =  channel->getTransmissionFinishTime();
	simtime_t delayToWait = 0;
	if (endTransmissionTime>now)
		delayToWait = endTransmissionTime - now;
	sendDelayed(msg,delayToWait,"out");
}

void MessageChecker::finish()
{
	if (forwardedMsg > checkedMsg)
		opp_error("%d message(s) has not been checked", forwardedMsg - checkedMsg);
	if (m_iterChk != m_checkingInfo.end())
		opp_error("several message(s) have to be checked");
}