using BearBuildTool.Projects;
using System.IO;
using System;
public class bear_render_null :Project
{
	public bear_render_null(string ProjectPath)
	{
		PCHFile=Path.Combine(ProjectPath,"source","NULLPCH.cpp");
		PCHIncludeFile="NULLPCH.h";
		AddSourceFiles(Path.Combine(ProjectPath,"source"),true);
		Projects.Private.Add("bear_core");
	}
} 