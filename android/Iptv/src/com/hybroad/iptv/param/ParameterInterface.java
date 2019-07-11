package com.hybroad.iptv.param;

public abstract interface ParameterInterface
{
  public abstract String getParameter(String name);

  public abstract boolean setParameter(String name, String value);
}
