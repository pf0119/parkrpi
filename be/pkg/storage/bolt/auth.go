package bolt

import (
	"github.com/asdine/storm"

	"toy/errors"
	"toy/pkg/auth"
	"toy/pkg/settings"
)

type authBackend struct {
	db *storm.DB
}

func (s authBackend) Get(t settings.AuthMethod) (auth.Auther, error) {
	var auther auth.Auther

	switch t {
	case auth.MethodJSONAuth:
		auther = &auth.JSONAuth{}
	case auth.MethodProxyAuth:
		auther = &auth.ProxyAuth{}
	case auth.MethodNoAuth:
		auther = &auth.NoAuth{}
	default:
		return nil, errors.ErrInvalidAuthMethod
	}

	return auther, get(s.db, "auther", auther)
}

func (s authBackend) Save(a auth.Auther) error {
	return save(s.db, "auther", a)
}
